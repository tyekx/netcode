#pragma once

#include "../GameObject.h"
#include <Netcode/Network/Connection.h>
#include <Netcode/Network/Service.h>
#include <Netcode/Network/ServerSession.h>
#include "NetwUtil.h"

struct ObjectState {
	Netcode::Float3 position;
	float yaw;
	float pitch;
};

struct TickRecord {
	Netcode::Timestamp timestamp;
	std::vector<ObjectState> states;
};

struct HistoryBuffer {
	std::vector<TickRecord> records;
};

class ServerClockSyncRequestFilter;

class GameServer {
	
	Ref<nn::ServerSession> serverSession;
	std::vector<ExtClientAction> actions;
	Ref<nn::NetcodeService> service;
	nn::ConnectionStorage * connections;
	std::unique_ptr<ServerClockSyncRequestFilter> clockSyncFilter;
	Netcode::GameClock gameClock;
	uint32_t nextGameObjectId;
	

	void HandleFireAction(const ExtClientAction & action);
	static void HandleMovementAction(const ExtClientAction & action);
	void HandleSpawnAction(const ExtClientAction & action);
	void DisconnectPlayer(Connection * connection);

	void FetchActions();
	
	void ProcessActions();

	void FetchControlMessages();

	void ProcessControlMessages();

	void SaveState();

	void CheckTimeouts();

	void BuildServerUpdates();

	void SendServerUpdates();
	
public:

	GameServer();
	~GameServer();
	
	void Tick();

	void Start(Netcode::Module::INetworkModule * network);
};
