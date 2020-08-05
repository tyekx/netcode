#pragma once 

#include <NetcodeFoundation/Version.h>
#include <Netcode/Modules.h>
#include <Netcode/Network/ServerSession.h>

#include <mutex>
#include <algorithm>

class PlayerStorage {
	mutable std::mutex mutex;

	struct User {
		std::string address;
		int32_t port;
		Netcode::Protocol::User state;
		Netcode::Network::UserRow userData;
		int32_t foreignControlId;
		int32_t localControlId;

		User(std::string addr, int32_t port, Netcode::Network::UserRow ud) :
			address{ std::move(addr) }, port{ port }, state{}, userData{ std::move(ud) }, foreignControlId{ 1 }, localControlId{ 1 } {
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

	void _NotifyPlayers_PlayerJoinedUnsafe(int32_t joinedPlayerId, std::string name) {
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

			serverSession->SendControlMessage(std::move(message),
				std::bind(&PlayerStorage::OnControlMessageSent, this, std::placeholders::_1, user.userData.userId));
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

			serverSession->SendControlMessage(std::move(message),
				std::bind(&PlayerStorage::OnControlMessageSent, this, std::placeholders::_1, user.userData.userId));
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

		serverSession->SendControlMessage(std::move(message),
			std::bind(&PlayerStorage::OnControlMessageSent, this, std::placeholders::_1, user.userData.userId));
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
		serverSession = sess;
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

		std::remove_if(std::begin(pendingPlayers), std::end(pendingPlayers), [userId](const Netcode::Network::UserRow & row) -> bool {
			return row.userId == userId;
		});
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

	void Disconnect(std::string addr, int32_t port, std::string hash) {
		std::scoped_lock<std::mutex> lock{ mutex };

		auto it = _FindUserByHashUnsafe(hash);

		if(it != std::end(activePlayers)) {
			if(it->address == addr && it->port == port) {
				Log::Debug("[Server] Successfully disconnected user: {0}...", hash.substr(0, 8));
				_NotifyPlayer_AcceptLeaveUnsafe(std::move(addr), port, it->localControlId++);
				int32_t playerId = it->userData.userId;
				activePlayers.erase(it);
				_NotifyPlayers_PlayerLeftUnsafe(playerId);
				return;
			}
		}

		// not okay branch
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
	Netcode::Network::Config config;
	Ref<Netcode::Network::ServerSession> serverSession;
	std::vector<Netcode::Protocol::Message> controlMessages;
	std::vector<Netcode::Protocol::Message> gameMessages;
	Netcode::Network::GameSessionRef clientSession;
	PlayerStorage players;
public:
	void Configure(Netcode::Network::Config cfg) {
		config = std::move(cfg);
	}

	/*
	Initialize modules
	*/
	virtual void Setup(Netcode::Module::IModuleFactory * factory) override {
		network = factory->CreateNetworkModule(this, 0);
		window = factory->CreateWindowModule(this, 1);

		StartModule(network.get());
		StartModule(window.get());

		auto ptr = network->CreateServer(config);
		serverSession = std::dynamic_pointer_cast<Netcode::Network::ServerSession>(ptr);
		players.SetSession(serverSession);
		serverSession->Start();

		config.client.serverHost = "localhost";
		config.client.localPort = 8887;
		config.client.serverControlPort = 8888;
		config.client.serverGamePort = 8889;
		config.client.tickIntervalMs = 500;

		clientSession = network->CreateClient(config);
		clientSession->Start();
		Netcode::Protocol::Message loginMessage;
		auto * ctrl = loginMessage.mutable_control();
		auto * req = ctrl->mutable_request();
		req->set_hash("f5e9cd4c2a36fc5844c4513dce9eb1e1b68a4e4e815eca92297af389ff5cd8fd");
		req->set_type(Netcode::Protocol::Control::RequestType::JOIN);
		auto * version = req->mutable_version();
		version->set_major(Netcode::GetMajorVersion());
		version->set_minor(Netcode::GetMinorVersion());
		version->set_build(Netcode::GetBuildVersion());

		clientSession->SendControlMessage(std::move(loginMessage), [](Netcode::Network::ErrorCode errorCode) ->void {
			if(errorCode) {
				Log::Error("[Client] server failed to ack login message");
			}
		});
	}

	/*
	Advance simulation, update modules
	*/
	virtual void Run() override {
		using c_t = std::chrono::high_resolution_clock;

		int isTested = 0;
		while(window->KeepRunning()) {
			window->ProcessMessages();
			auto begin = c_t::now();
			
			while(std::chrono::duration_cast<std::chrono::milliseconds>(c_t::now() - begin).count() < config.server.tickIntervalMs) {
				std::this_thread::sleep_for(std::chrono::microseconds(500));
			}

			serverSession->Receive(controlMessages, gameMessages);
			ProcessControlMessages();
			ProcessGameMessages();
			serverSession->SendAll();

			if(isTested != 5) {
				isTested += 1;
			} else {
				isTested += 1;
				Log::Debug("Testing leave functionality");
				Netcode::Protocol::Message loginMessage;
				auto * ctrl = loginMessage.mutable_control();
				auto * req = ctrl->mutable_request();
				req->set_hash("f5e9cd4c2a36fc5844c4513dce9eb1e1b68a4e4e815eca92297af389ff5cd8fd");
				req->set_type(Netcode::Protocol::Control::RequestType::LEAVE);
				auto * version = req->mutable_version();
				version->set_major(Netcode::GetMajorVersion());
				version->set_minor(Netcode::GetMinorVersion());
				version->set_build(Netcode::GetBuildVersion());
				clientSession->SendControlMessage(std::move(loginMessage), [](Netcode::Network::ErrorCode errorCode) ->void {
					if(errorCode) {
						Log::Error("[Client] server failed to ack leave message");
					}
				});
			}
		}
	}

	void OnSessionCreated(Netcode::Network::ErrorCode errorCode, int32_t userId, std::string address, int32_t port) {
		if(errorCode) {
			Log::Debug("[Server] Failed to create game session: {0}", errorCode.message());
			players.CancelConnection(userId);
			DeclinePlayerJoinRequest(std::move(address), port, "Already have an active session.");
			return;
		}

		if(players.AcceptConnection(userId, std::move(address), port)) {
			Log::Debug("[Server] Session successfully created");
		} else {
			Log::Debug("[Server] AcceptConnection failed, rolling back");
			DeclinePlayerJoinRequest(std::move(address), port, "Something went wrong.");
			serverSession->TerminateGameSession(userId, [](Netcode::Network::ErrorCode ec)-> void {
				if(ec) {
					Log::Error("[Server] Failed to terminate game session properly");
				} else {
					Log::Info("[Server] Cleaned up game sessions");
				}
			});
		}
	}

	void OnAuthenticated(Netcode::Network::ErrorCode errorCode, Netcode::Network::UserRow userData, std::string address, int32_t port) {
		if(errorCode) {
			Log::Debug("[Server] Failed to authenticate user: {0}", errorCode.message());
			DeclinePlayerJoinRequest(std::move(address), port, "Authentication failed.");
			return;
		}

		if(userData.isBanned) {
			Log::Debug("[Server] User is banned.");
			DeclinePlayerJoinRequest(std::move(address), port, "Your account is banned.");
			return;
		}

		int32_t userId = userData.userId;

		if(players.StartConnection(std::move(userData), config.server.playerSlots)) {
			serverSession->CreateGameSession(userId, std::bind(&ServerApp::OnSessionCreated, this,
				std::placeholders::_1,
				userId,
				address,
				port));
		} else {
			DeclinePlayerJoinRequest(std::move(address), port, "Already connected to the server");
		}
	}

	void DeclinePlayerJoinRequest(std::string addr, int32_t port, std::string reason) {
		using Netcode::Protocol::Control::RequestType;
		Netcode::Protocol::Message message;
		message.set_id(1);
		message.set_address(std::move(addr));
		message.set_port(port);
		auto *ctrl = message.mutable_control();
		auto * response = ctrl->mutable_response();
		response->set_type(RequestType::JOIN);
		response->set_note(std::move(reason));

		serverSession->SendControlMessage(std::move(message), [](Netcode::Network::ErrorCode ec) ->void {
			if(ec) {
				Log::Error("[Server] Client failed to ACK declined join message");
			}
		});
	}

	void DeclinePlayerLeaveRequest(std::string addr, int32_t port, std::string reason) {
		using Netcode::Protocol::Control::RequestType;
		Netcode::Protocol::Message message;
		message.set_id(0);
		message.set_address(std::move(addr));
		message.set_port(port);
		auto * ctrl = message.mutable_control();
		auto * response = ctrl->mutable_response();
		response->set_type(RequestType::LEAVE);
		response->set_note(std::move(reason));

		serverSession->SendControlMessage(std::move(message), [](Netcode::Network::ErrorCode ec) ->void {
			if(ec) {
				Log::Error("[Server] Client failed to ACK declined join message");
			}
		});
	}

	void HandleJoinRequest(const Netcode::Protocol::Message & message) {
		using Netcode::Protocol::Control::Request;
		const Request & request = message.control().request();

		if(!request.has_version()) {
			Log::Debug("[Server] Client has not sent the version data");
			DeclinePlayerJoinRequest(message.address(), message.port(), "Missing version data.");
			return;
		}

		if(!serverSession->CheckVersion(request.version())) {
			Log::Debug("[Server] Client has a mismatching version");
			DeclinePlayerJoinRequest(message.address(), message.port(), "Server has a mismatching version.");
			return;
		}

		if(players.GetNumPlayers() >= config.server.playerSlots) {
			Log::Debug("[Server] can not join, server is full");
			DeclinePlayerJoinRequest(message.address(), message.port(), "Server is full.");
			return;
		}

		std::string hash = request.hash();

		if(hash.empty()) {
			Log::Debug("[Server] user sent a malformed join request");
			DeclinePlayerJoinRequest(message.address(), message.port(), "Malformed join request (missing hash).");
			return;
		}

		serverSession->Authenticate(hash, std::bind(&ServerApp::OnAuthenticated, this,
			std::placeholders::_1,
			std::placeholders::_2,
			message.address(),
			message.port()));
	}
	
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
