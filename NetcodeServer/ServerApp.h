#pragma once 

#include <Netcode/Modules.h>
#include <Netcode/Network/ServerSession.h>
#include <Netcode/Config.h>
#include <Netcode/System/SystemClock.h>

#include <mutex>
#include <algorithm>

#include <grpcpp/grpcpp.h>
#include "RpcService.h"
#include "RpcServer.h"

class ServerApp : public Netcode::Module::AApp {
	RpcServer rpcServer;
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
		for(Netcode::Protocol::ClientUpdate & cu : gameMessages) {
			Netcode::Protocol::ServerUpdate resp;
			if(cu.has_time_sync()) {
				resp.set_allocated_time_sync(cu.release_time_sync());
			}
			serverSession->Update(1, std::move(resp));
		}
		
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
