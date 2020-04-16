#pragma once 

#include <Netcode/Modules.h>
#include <Netcode/Network/ServerSession.h>

class PlayerStorage {
public:
	void StartConnection(Netcode::Network::UserRow userData) {

	}

	void CancelConnection(int32_t userId) {

	}

	void AcceptConnection(int32_t userId, std::string address, int32_t port) {

	}

	// pendingConnections + acceptedConnections
	uint32_t GetNumPlayers() const {
		return 0;
	}
};

class ServerApp : public Netcode::Module::AApp {
	Netcode::Network::Config config;
	std::shared_ptr<Netcode::Network::ServerSession> serverSession;
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

		config.client.serverHost = "localhost";
		config.client.localPort = 8887;
		config.client.serverControlPort = 8888;
		config.client.serverGamePort = 8889;
		config.client.tickIntervalMs = 500;

		clientSession = network->CreateClient(config);
		Netcode::Protocol::Message loginMessage;
		loginMessage.set_id(1);
		auto * ctrl = loginMessage.mutable_control();
		auto * req = ctrl->mutable_request();
		req->set_hash("f5e9cd4c2a36fc5844c4513dce9eb1e1b68a4e4e815eca92297af389ff5cd8fd");
		req->set_type(Netcode::Protocol::Control::RequestType::JOIN);
		req->set_version("0.0.1");

		clientSession->SendControlMessage(std::move(loginMessage));
	}

	/*
	Advance simulation, update modules
	*/
	virtual void Run() override {
		using c_t = std::chrono::high_resolution_clock;
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
		}
	}

	void OnSessionCreated(Netcode::Network::ErrorCode errorCode, int32_t userId, std::string address, int32_t port) {
		if(errorCode) {
			Log::Debug("[Server] Failed to create game session: {0}", errorCode.message());
			players.CancelConnection(userId);
			return;
		}

		Log::Debug("Session successfully created");

		players.AcceptConnection(userId, std::move(address), port);
	}

	void OnAuthenticated(Netcode::Network::ErrorCode errorCode, Netcode::Network::UserRow userData, std::string address, int32_t port) {
		if(errorCode) {
			Log::Debug("[Server] Failed to authenticate user: {0}", errorCode.message());
			return;
		}

		if(userData.isBanned) {
			Log::Debug("[Server] User is banned.");
			return;
		}

		int32_t userId = userData.userId;

		players.StartConnection(std::move(userData));

		serverSession->CreateGameSession(userId, std::bind(&ServerApp::OnSessionCreated, this,
			std::placeholders::_1,
			userId,
			address,
			port));
	}

	void HandleJoinRequest(const Netcode::Protocol::Message & message) {
		if(players.GetNumPlayers() >= config.server.playerSlots) {
			Log::Debug("[Server] can not join, server is full");
			// failed request
			return;
		}

		std::string hash = message.control().request().hash();

		if(hash.empty()) {
			// failed request
			Log::Debug("[Server] user sent a malformed join request");
			return;
		}

		serverSession->Authenticate(hash, std::bind(&ServerApp::OnAuthenticated, this,
			std::placeholders::_1,
			std::placeholders::_2,
			message.address(),
			message.port()));
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
		ShutdownModule(network.get());
		ShutdownModule(window.get());
	}
};
