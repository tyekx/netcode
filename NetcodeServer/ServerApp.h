#pragma once 

#include <Netcode/Modules.h>
#include <Netcode/Network/ServerSession.h>
#include <Netcode/Config.h>
#include <Netcode/System/System.h>
#include <Netcode/System/SystemClock.h>

class ServerApp : public Netcode::Module::AApp {
	Ref<Netcode::Network::ServerSession> serverSession;
	std::vector<Netcode::Protocol::ClientUpdate> gameMessages;
	Ref<Netcode::Network::ClientSessionBase> clientSession;
public:
	/*
	Initialize modules
	*/
	virtual void Setup(Netcode::Module::IModuleFactory * factory) override;

	/*
	Advance simulation, update modules
	*/
	virtual void Run() override;

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
