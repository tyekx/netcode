#pragma once 

#include <Netcode/Modules.h>
#include <Netcode/Network/ServerSession.h>

class ServerApp : public Netcode::Module::AApp {
	Netcode::Network::Config config;
	std::shared_ptr<Netcode::Network::ServerSession> serverSession;
	std::vector<Netcode::Protocol::Message> controlMessages;
	std::vector<Netcode::Protocol::Message> gameMessages;
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

		auto ptr = network->CreateServer(config);
		serverSession = std::dynamic_pointer_cast<Netcode::Network::ServerSession>(ptr);
	}

	/*
	Advance simulation, update modules
	*/
	virtual void Run() override {
		using c_t = std::chrono::high_resolution_clock;
		while(window->KeepRunning()) {
			auto begin = c_t::now();
			
			while(std::chrono::duration_cast<std::chrono::milliseconds>(c_t::now() - begin).count() < config.server.tickIntervalMs) {
				std::this_thread::sleep_for(std::chrono::microseconds(500));
			}
			
			serverSession->Receive(controlMessages, gameMessages);
			// process
			// send

		}
	}

	/*
	Properly shutdown the application
	*/
	virtual void Exit() override {
		ShutdownModule(network.get());
	}
};
