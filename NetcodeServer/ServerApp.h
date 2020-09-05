#pragma once 

#include <Netcode/Modules.h>
#include <Netcode/Network/ServerSession.h>
#include <Netcode/Config.h>

#include <mutex>
#include <algorithm>

#include <grpcpp/grpcpp.h>
#include "RpcService.h"

class PlayerStorage {
	mutable std::mutex mutex;

	struct User {
		std::string address;
		int32_t port;
		Netcode::Protocol::User state;
		Netcode::Network::UserRow userData;
		int32_t foreignControlId;
		int32_t localControlId;

		User(std::string addr, int32_t portNum, Netcode::Network::UserRow ud) :
			address{ std::move(addr) }, port{ portNum }, state{}, userData{ std::move(ud) }, foreignControlId{ 1 }, localControlId{ 1 } {
			state.set_id(userData.userId);
			state.set_state(Netcode::Protocol::PlayerState::OBSERVER);
		}
	};

	Ref<Netcode::Network::ServerSession> serverSession;
	std::list<User> activePlayers;
	std::vector<Netcode::Network::UserRow> pendingPlayers;

	uint32_t _GetNumPlayersUnsafe() const {
		return static_cast<uint32_t>(activePlayers.size()) +
			static_cast<uint32_t>(pendingPlayers.size());
	}

	void OnControlMessageSent(Netcode::Network::ErrorCode errorCode, int32_t userId) {
		if(errorCode) {
			// timed out
			Log::Error("[Server] control message was not acknowledged, dropping user");
			NotifyPlayers_PlayerTimedOut(userId);
		}
	}

	void _NotifyPlayers_PlayerJoinedUnsafe(int32_t joinedPlayerId, const std::string& name) {
		using Netcode::Protocol::Control::CommandType;

		for(User & user : activePlayers) {
			Netcode::Protocol::Message message;
			message.set_address(user.address);
			message.set_port(user.port);
			message.set_id(user.localControlId++);

			Netcode::Protocol::Control::Message * ctrl = message.mutable_control();
			Netcode::Protocol::Control::Command * command = ctrl->mutable_command();

			command->set_type(CommandType::PLAYER_JOINED);
			Netcode::Protocol::User * newUser = command->mutable_user();
			newUser->set_id(joinedPlayerId);
			newUser->set_state(Netcode::Protocol::PlayerState::OBSERVER);
			newUser->set_name(name);

			int32_t uId = user.userData.userId;

			serverSession->SendControlMessage(std::move(message), [this, uId](Netcode::Network::ErrorCode ec) -> void {
				OnControlMessageSent(ec, uId);
			});
		}
	}

	void _NotifyPlayers_PlayerLeftUnsafe(int32_t leftPlayerId) {
		using Netcode::Protocol::Control::CommandType;

		for(User & user : activePlayers) {
			Netcode::Protocol::Message message;
			message.set_address(user.address);
			message.set_port(user.port);
			message.set_id(user.localControlId++);

			Netcode::Protocol::Control::Message * ctrl = message.mutable_control();
			Netcode::Protocol::Control::Command * command = ctrl->mutable_command();

			command->set_type(CommandType::PLAYER_TIMEDOUT);
			Netcode::Protocol::User * newUser = command->mutable_user();
			newUser->set_id(leftPlayerId);

			int32_t uId = user.userData.userId;

			serverSession->SendControlMessage(std::move(message), [this, uId](Netcode::Network::ErrorCode ec) -> void {
				OnControlMessageSent(ec, uId);
			});
		}
	}

	void _NotifyPlayers_PlayerTimedOutUnsafe(int32_t playerId) {
		using Netcode::Protocol::Control::CommandType;

		for(User & user : activePlayers) {
			Netcode::Protocol::Message message;
			message.set_address(user.address);
			message.set_port(user.port);
			message.set_id(user.localControlId++);

			Netcode::Protocol::Control::Message * ctrl = message.mutable_control();
			Netcode::Protocol::Control::Command * command = ctrl->mutable_command();

			command->set_type(CommandType::PLAYER_TIMEDOUT);
			Netcode::Protocol::User * newUser = command->mutable_user();
			newUser->set_id(playerId);

			if(user.userData.userId != playerId) {
				serverSession->SendControlMessage(std::move(message),
					std::bind(&PlayerStorage::OnControlMessageSent, this, std::placeholders::_1, user.userData.userId));
			}
		}
	}

	void _NotifyPlayer_JoinAcceptedUnsafe(User & user) {
		Netcode::Protocol::Message message;
		message.set_id(user.localControlId++);
		message.set_address(user.address);
		message.set_port(user.port);
		auto * userJoined = message.mutable_control()->mutable_response()->mutable_user_joined();
		(*userJoined) = user.state;

		int32_t uId = user.userData.userId;

		serverSession->SendControlMessage(std::move(message), [this, uId](Netcode::Network::ErrorCode ec) ->void
		{
			OnControlMessageSent(ec, uId);
		});
	}

	void _NotifyPlayer_AcceptLeaveUnsafe(std::string addr, int32_t port, int32_t messageId) {
		Netcode::Protocol::Message message;
		message.set_id(messageId);
		message.set_address(std::move(addr));
		message.set_port(port);
		auto * userLeft = message.mutable_control()->mutable_response();
		userLeft->set_type(Netcode::Protocol::Control::RequestType::LEAVE);
	}

	void _NotifyPlayer_DeclineLeaveUnsafe(std::string addr, int32_t port, int32_t messageId, std::string reason) {
		Netcode::Protocol::Message message;
		message.set_id(messageId);
		message.set_address(std::move(addr));
		message.set_port(port);
		auto * userLeft = message.mutable_control()->mutable_response();
		userLeft->set_type(Netcode::Protocol::Control::RequestType::LEAVE);
		userLeft->set_note(std::move(reason));

		serverSession->SendControlMessage(std::move(message), [](Netcode::Network::ErrorCode errorCode) -> void {
			Log::Info("[Server] Player left and failed to ack the response");
		});
	}

	std::list<User>::iterator _FindUserByHashUnsafe(const std::string & hash) {
		return std::find_if(std::begin(activePlayers), std::end(activePlayers), [&hash](const User & player) -> bool {
			return player.userData.hash == hash;
		});
	}

public:
	void SetSession(Ref<Netcode::Network::ServerSession> sess) {
		serverSession = std::move(sess);
	}

	void NotifyPlayers_PlayerTimedOut(int32_t userId) {
		std::scoped_lock<std::mutex> lock{ mutex };
		_NotifyPlayers_PlayerTimedOutUnsafe(userId);
	}

	bool StartConnection(Netcode::Network::UserRow userData, uint32_t playerLimit) {
		std::scoped_lock<std::mutex> lock{ mutex };

		if(_GetNumPlayersUnsafe() >= playerLimit) {
			return false;
		}

		for(const auto & i : pendingPlayers) {
			if(i.userId == userData.userId) {
				return false;
			}
		}

		pendingPlayers.push_back(std::move(userData));

		return true;
	}

	void CancelConnection(int32_t userId) {
		std::scoped_lock<std::mutex> lock{ mutex };

		auto it = std::remove_if(std::begin(pendingPlayers), std::end(pendingPlayers),
		               [userId](const Netcode::Network::UserRow& row) -> bool
		               {
			               return row.userId == userId;
		               });

		pendingPlayers.erase(it);
	}

	void NotifyPlayer_JoinAccepted(int32_t userId) {
		std::scoped_lock<std::mutex> lock{ mutex };

		for(auto & player : activePlayers) {
			if(player.userData.userId == userId) {
				Log::Debug("[Server] Sending join accepted message");
				_NotifyPlayer_JoinAcceptedUnsafe(player);
				break;
			}
		}
	}

	void Disconnect(std::string addr, int32_t port, const std::string & hash) {
		std::scoped_lock<std::mutex> lock{ mutex };

		auto it = _FindUserByHashUnsafe(hash);

		if(it != std::end(activePlayers)) {
			if(it->address == addr && it->port == port) {
				Log::Debug("[Server] Successfully disconnected user: {0}...", hash.substr(0, 8));
				_NotifyPlayer_AcceptLeaveUnsafe(std::move(addr), port, it->localControlId++);
				int32_t uId = it->userData.userId;
				activePlayers.erase(it);
				_NotifyPlayers_PlayerLeftUnsafe(uId);
				return;
			}
		}

		_NotifyPlayer_DeclineLeaveUnsafe(std::move(addr), port, 0, "Player is not connected.");
	}

	bool AcceptConnection(int32_t userId, std::string address, int32_t port) {
		std::scoped_lock<std::mutex> lock{ mutex };

		for(const auto & i : activePlayers) {
			if(i.userData.userId == userId) {
				return false;
			}
		}

		auto it = std::find_if(std::begin(pendingPlayers), std::end(pendingPlayers), [userId](const Netcode::Network::UserRow & row) -> bool {
			return row.userId == userId;
		});

		if(it == std::end(pendingPlayers)) {
			return false;
		}

		_NotifyPlayers_PlayerJoinedUnsafe(it->userId, it->name);

		_NotifyPlayer_JoinAcceptedUnsafe(activePlayers.emplace_back(std::move(address), port, std::move(*it)));

		pendingPlayers.erase(it);

		return true;
	}

	uint32_t GetNumPlayers() const {
		std::scoped_lock<std::mutex> lock{ mutex };
		return _GetNumPlayersUnsafe();
	}
};

class ServerApp : public Netcode::Module::AApp {
	std::unique_ptr<Netcode::Protocol::ServerService::Service> rpcService;
	std::unique_ptr<grpc_impl::Server> rpcServer;
	Ref<Netcode::Network::ServerSession> serverSession;
	std::vector<Netcode::Protocol::Message> controlMessages;
	std::vector<Netcode::Protocol::Message> gameMessages;
	Netcode::Network::GameSessionRef clientSession;
	PlayerStorage players;
public:
	/*
	Initialize modules
	*/
	virtual void Setup(Netcode::Module::IModuleFactory * factory) override;

	/*
	Advance simulation, update modules
	*/
	virtual void Run() override;

	void OnSessionCreated(Netcode::Network::ErrorCode errorCode, int32_t userId, std::string address, int32_t port);

	void OnAuthenticated(Netcode::Network::ErrorCode errorCode, Netcode::Network::UserRow userData, std::string address, int32_t port);

	void DeclinePlayerJoinRequest(std::string addr, int32_t port, std::string reason);

	void DeclinePlayerLeaveRequest(std::string addr, int32_t port, std::string reason);

	void HandleJoinRequest(const Netcode::Protocol::Message & message);
	
	void HandleRespawnRequest(const Netcode::Protocol::Message & message) {

	}

	void HandleLeaveRequest(const Netcode::Protocol::Message & message) {
		using Netcode::Protocol::Control::Request;
		const Request & request = message.control().request();

		std::string hash = request.hash();

		if(hash.empty()) {
			Log::Debug("[Server] user sent a malformed join request");
			DeclinePlayerLeaveRequest(message.address(), message.port(), "Malformed leave request (missing hash).");
			return;
		}

		Log::Debug("[Server] Trying to disconnect user: {0}", hash.substr(0, 8));
		players.Disconnect(message.address(), message.port(), std::move(hash));
	}

	void ProcessControlMessages() {
		using Netcode::Protocol::Control::RequestType;

		for(const Netcode::Protocol::Message & m : controlMessages) {
			const Netcode::Protocol::Control::Request & request = m.control().request();

			RequestType type = request.type();
			switch(type) {
				case RequestType::JOIN:
					HandleJoinRequest(m);
					break;
				case RequestType::LEAVE:
					HandleLeaveRequest(m);
					break;
				case RequestType::RESPAWN:
					HandleRespawnRequest(m);
					break;
				default:
					Log::Debug("[Server] Unhandled control message");
					break;
			}
		}

		controlMessages.clear();
	}

	void ProcessGameMessages() {
		gameMessages.clear();


	}

	/*
	Properly shutdown the application
	*/
	virtual void Exit() override {
		serverSession->Stop();
		clientSession->Stop();

		serverSession.reset();
		clientSession.reset();

		ShutdownModule(network.get());
		ShutdownModule(window.get());
	}
};
