#pragma once

#include "../GameObject.h"
#include <Netcode/Network/Connection.h>
#include <Netcode/Network/Service.h>
#include <Netcode/Network/ServerSession.h>
#include "NetwUtil.h"

class GameServer {
	Ref<nn::ServerSession> serverSession;
	std::vector<ExtClientAction> actions;
	Ref<nn::NetcodeService> service;
	nn::ConnectionStorage * connections;
	Netcode::GameClock gameClock;
	uint32_t nextGameObjectId;

	void HandleFireAction(const ExtClientAction & action);
	static void HandleMovementAction(const ExtClientAction & action);
	static void HandleLookAction(const ExtClientAction & action);
	void HandleSpawnAction(const ExtClientAction & action);
	void DisconnectPlayer(Connection * connection);

	void FetchActions();
	
	void ProcessActions();

	void FetchControlMessages() {
		
	}

	void ProcessControlMessages() {
		
	}

	void CheckTimeouts();

	void BuildServerUpdates();

	void SendServerUpdates();
	
public:

	void Tick();

	void Start(Netcode::Module::INetworkModule * network);
};
