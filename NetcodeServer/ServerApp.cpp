#include "ServerApp.h"


/*
Initialize modules
*/

std::unique_ptr<std::thread> t;

void ServerApp::Setup(Netcode::Module::IModuleFactory * factory) {
	network = factory->CreateNetworkModule(this, 0);
	window = factory->CreateWindowModule(this, 1);
	
	StartModule(network.get());
	StartModule(window.get());

	auto ptr = network->CreateServer();
	serverSession = std::dynamic_pointer_cast<Netcode::Network::ServerSession>(ptr);
	serverSession->Start();

	rpcServer.ss = serverSession;
	rpcServer.Start();

	t = std::make_unique<std::thread>([this]() ->void {
		Sleep(500);

		auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());

		auto stub = Netcode::Protocol::ServerService::NewStub(channel);

		grpc::ClientContext ctx;
		Netcode::Protocol::Request req;

		ctx.AddMetadata("x-netcode-auth", "de14a05b71fc3910cc34998f358d9ce1d7012e8919093f8817f7cf63507748b8");
		req.set_type(Netcode::Protocol::RequestType::CONNECT);
		Netcode::Protocol::Response resp;
		grpc::Status st = stub->Connect(&ctx, req, &resp);

		Log::Debug("Received nonce: {0}", resp.connect_data().nonce());
		Log::Debug(" Received port: {0}", resp.connect_data().port());
		
		/*Sleep(3000);
		grpc::ClientContext ctx2;
		ctx2.AddMetadata("x-netcode-auth", "de14a05b71fc3910cc34998f358d9ce1d7012e8919093f8817f7cf63507748b8");
		req.set_type(Netcode::Protocol::RequestType::DISCONNECT);
		Netcode::Protocol::Response disconnectResp;
		grpc::Status dst = stub->Disconnect(&ctx2, req, &disconnectResp);
		std::cout << "Disconnect: " << dst.error_message() << std::endl;*/

		clientSession = network->CreateClient();
		clientSession->Start();
		clientSession->Connect("::1", resp.connect_data().port(), resp.connect_data().nonce());
	});

	/*
	clientSession = network->CreateClient();
	clientSession->Start();

	clientSession->Connect("::1", 8889, "cookiehash");*/
}

/*
Advance simulation, update modules
*/

void ServerApp::Run() {
	uint32_t tickIntervalMs = Netcode::Config::Get<uint32_t>(L"network.server.tickIntervalMs:u32");

	Netcode::Duration frameDuration = std::chrono::milliseconds(tickIntervalMs);
	Netcode::Timestamp frameStartedAt = Netcode::SystemClock::LocalNow();
	
	while(window->KeepRunning()) {
		serverSession->SwapBuffers(gameMessages);
		ProcessGameMessages();

		const Netcode::Timestamp frameFinishedAt = Netcode::SystemClock::LocalNow();
		const Netcode::Duration sim = frameFinishedAt - frameStartedAt;
		frameStartedAt = frameFinishedAt;

		if(sim < frameDuration) {
			Netcode::BusyWait(frameDuration - sim, [this]() -> void {
				window->ProcessMessages();
			});
		}
	}
}
