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
	serverSession->Start();

	clientSession = network->CreateClient();
	clientSession->Start();
	//clientSession->Connect("::1", 9999, "");
}

/*
Advance simulation, update modules
*/

void ServerApp::Run() {
	uint32_t tickIntervalMs = Netcode::Config::Get<uint32_t>(L"network.server.tickIntervalMs:u32");

	Netcode::Duration frameDuration = std::chrono::milliseconds(tickIntervalMs);
	Netcode::Timestamp frameStartedAt = Netcode::SystemClock::LocalNow();
	
	while(window->KeepRunning()) {
		//serverSession->SwapBuffers(gameMessages);
		ProcessGameMessages();

		const Netcode::Timestamp frameFinishedAt = Netcode::SystemClock::LocalNow();
		const Netcode::Duration sim = frameFinishedAt - frameStartedAt;
		frameStartedAt = frameFinishedAt;

		if(sim < frameDuration) {
			Netcode::SleepFor(frameDuration - sim);
			/*
			Netcode::BusyWait(frameDuration - sim, [this]() -> void {
				window->ProcessMessages();
			});*/
		}
	}
}
