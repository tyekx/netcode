#include "ServerApp.h"

/*
Initialize modules
*/

void ServerApp::Setup(Netcode::Module::IModuleFactory * factory) {
	network = factory->CreateNetworkModule(this, 0);
	window = factory->CreateWindowModule(this, 1);

	StartModule(network.get());
	StartModule(window.get());

	auto ptr = network->CreateServer();
	serverSession = std::dynamic_pointer_cast<Netcode::Network::ServerSession>(ptr);
	players.SetSession(serverSession);
	serverSession->Start();

	grpc::EnableDefaultHealthCheckService(true);

	rpcService = std::make_unique<RpcServiceImpl>();

	grpc::ServerBuilder builder;
	int portNum;
	builder.RegisterService(rpcService.get());
	builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials(), &portNum);
	rpcServer = builder.BuildAndStart();

	Log::Debug("gRPC port: {0}", portNum);

	clientSession = network->CreateClient();
	clientSession->Start();
	Netcode::Protocol::Message loginMessage;
	auto * ctrl = loginMessage.mutable_control();
	auto * req = ctrl->mutable_request();
	req->set_hash("f5e9cd4c2a36fc5844c4513dce9eb1e1b68a4e4e815eca92297af389ff5cd8fd");
	req->set_type(Netcode::Protocol::Control::RequestType::JOIN);
	auto * version = req->mutable_version();
	version->set_major(0);
	version->set_minor(0);
	version->set_build(0);

	clientSession->SendControlMessage(std::move(loginMessage), [](Netcode::Network::ErrorCode errorCode) ->void {
		if(errorCode) {
			Log::Error("[Client] server failed to ack login message");
		}
	});
}

/*
Advance simulation, update modules
*/

void ServerApp::Run() {
	using c_t = std::chrono::high_resolution_clock;

	uint32_t tickIntervalMs = Netcode::Config::Get<uint32_t>("network.server.tickIntervalMs:u32");

	int isTested = 0;
	while(window->KeepRunning()) {
		window->ProcessMessages();
		auto begin = c_t::now();

		while(std::chrono::duration_cast<std::chrono::milliseconds>(c_t::now() - begin).count() < tickIntervalMs) {
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
			version->set_major(0);
			version->set_minor(0);
			version->set_build(0);
			clientSession->SendControlMessage(std::move(loginMessage), [](Netcode::Network::ErrorCode errorCode) ->void {
				if(errorCode) {
					Log::Error("[Client] server failed to ack leave message");
				}
			});
		}
	}
}

void ServerApp::OnSessionCreated(Netcode::Network::ErrorCode errorCode, int32_t userId, std::string address, int32_t port) {
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

void ServerApp::OnAuthenticated(Netcode::Network::ErrorCode errorCode, Netcode::Network::UserRow userData, std::string address, int32_t port) {
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

	//if(players.StartConnection(std::move(userData), config.server.playerSlots)) {
	if(players.StartConnection(std::move(userData), 10)) {
		serverSession->CreateGameSession(userId, std::bind(&ServerApp::OnSessionCreated, this,
			std::placeholders::_1,
			userId,
			address,
			port));
	} else {
		DeclinePlayerJoinRequest(std::move(address), port, "Already connected to the server");
	}
}

void ServerApp::DeclinePlayerJoinRequest(std::string addr, int32_t port, std::string reason) {
	using Netcode::Protocol::Control::RequestType;
	Netcode::Protocol::Message message;
	message.set_id(1);
	message.set_address(std::move(addr));
	message.set_port(port);
	auto * ctrl = message.mutable_control();
	auto * response = ctrl->mutable_response();
	response->set_type(RequestType::JOIN);
	response->set_note(std::move(reason));

	serverSession->SendControlMessage(std::move(message), [](Netcode::Network::ErrorCode ec) ->void {
		if(ec) {
			Log::Error("[Server] Client failed to ACK declined join message");
		}
	});
}

void ServerApp::DeclinePlayerLeaveRequest(std::string addr, int32_t port, std::string reason) {
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

void ServerApp::HandleJoinRequest(const Netcode::Protocol::Message & message) {
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

	if(players.GetNumPlayers() >= Netcode::Config::Get<uint8_t>("network.server.playerSlots:u8")) {
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
