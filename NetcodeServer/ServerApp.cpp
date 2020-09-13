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

	t = std::make_unique<std::thread>([]() ->void {
		Sleep(500);

		auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());

		auto stub = Netcode::Protocol::ServerService::NewStub(channel);

		grpc::ClientContext ctx;
		Netcode::Protocol::Request req;

		ctx.AddMetadata("x-netcode-auth", "f5e9cd4c2a36fc5844c4513dce9eb1e1b68a4e4e815eca92297af389ff5cd8fd");
		req.set_type(Netcode::Protocol::RequestType::CONNECT);
		Netcode::Protocol::Response resp;
		grpc::Status st = stub->Connect(&ctx, req, &resp);
		
		Log::Info("Done");
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
	using c_t = std::chrono::high_resolution_clock;

	uint32_t tickIntervalMs = Netcode::Config::Get<uint32_t>(L"network.server.tickIntervalMs:u32");

	while(window->KeepRunning()) {
		window->ProcessMessages();
		auto begin = c_t::now();

		while(std::chrono::duration_cast<std::chrono::milliseconds>(c_t::now() - begin).count() < tickIntervalMs) {
			std::this_thread::sleep_for(std::chrono::microseconds(500));
		}

		serverSession->SwapBuffers(gameMessages);
		ProcessGameMessages();
	}
}
