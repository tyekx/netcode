#pragma once

#include "../GameObject.h"
#include <Netcode/Network/Connection.h>
#include <Netcode/Network/Service.h>
#include <Netcode/Network/ServerSession.h>
#include "NetwUtil.h"
#include <random>

class ServerClockSyncRequestFilter;
class ServerConnRequestFilter;

struct SpawnPoint {
	Netcode::Float3 position;
	Netcode::Float4 rotation;
};

struct PerfData {
	Netcode::Timestamp timestamp;
	Netcode::Duration frameTime;
	Netcode::Duration receiveTime;
	Netcode::Duration parseTime;
	Netcode::Duration processTime;
	Netcode::Duration movementTime;
	Netcode::Duration reconstrTime;
	Netcode::Duration posCalcTime;
	Netcode::Duration pxSceneManipTime;
	Netcode::Duration pxScenePoseTime;
	uint32_t numShots;
	uint32_t numPxManip;
	uint32_t numPxPose;
	uint32_t numPosCalc;

	PerfData() : timestamp{}, frameTime{}, receiveTime{}, parseTime{}, processTime{}, movementTime{}, reconstrTime{},
		posCalcTime{}, pxSceneManipTime{}, pxScenePoseTime{}, numShots{}, numPxManip{}, numPxPose{}, numPosCalc{} {
		
	}
};

class GameServer {
	Ref<nn::ServerSession> serverSession;
	std::vector<ExtClientAction> actions;
	std::vector<SpawnPoint> spawnPoints;
	Ref<nn::NetcodeService> service;
	Netcode::PxPtr<physx::PxControllerManager> controllerManager;
	GameScene * gameScene;
	physx::PxScene * pxScene;
	nn::ConnectionStorage * connections;
	Netcode::GameClock gameClock;
	GameObject * scoreboardObject;
	ScoreboardScript * scoreboard;
	Netcode::Duration scoreboardReplInterval;
	std::mt19937 mersenneTwister;
	std::uniform_int_distribution<int> uniformIntDistribution;
	std::vector<PerfData> perf;
	uint32_t nextGameObjectId;

	void OnPlayerJoined(Connection * connection);
	void OnPlayerConnected(Connection * connection);

	void HandleFireAction(const ExtClientAction & action);
	void HandleMovementAction(const ExtClientAction & action);
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
	
	void Tick();

	void Start(Netcode::Module::INetworkModule * network);

	friend class ServerConnRequestFilter;
	friend class ServerClockSyncRequestFilter;
};
