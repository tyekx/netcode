#include "GameServer.h"
#include "../Services.h"
#include "../GameScene.h"
#include <Netcode/Network/Service.h>
#include <Netcode/Network/ClientSession.h>
#include <Netcode/Stopwatch.h>
#include <sstream>
#include <fstream>

using Netcode::Float3;
using Netcode::Vector3;
using Netcode::Quaternion;

static PerfData perfCurrent{};

class ServerConnRequestFilter : public nn::FilterBase {
public:
	GameServer * server;

	ServerConnRequestFilter(GameServer * srv) : server{srv} {}
	
	nn::FilterResult Run(Ptr<nn::NetcodeService> service, Ptr<nn::DtlsRoute> route, nn::ControlMessage & cm) override {
		const np::Control * peerControl = cm.control;
		if(peerControl->type() != np::CONNECT_REQUEST || !peerControl->has_connect_request()) {
			return nn::FilterResult::IGNORED;
		}

		const np::ConnectRequest * connReq = &peerControl->connect_request();

		if(route == nullptr || route->state != nn::DtlsRouteState::ESTABLISHED) {
			return nn::FilterResult::CONSUMED;
		}

		nn::ConnectionStorage * connections = service->GetConnections();

		if(connections->GetConnectionByEndpoint(cm.packet->endpoint) != nullptr) {
			return nn::FilterResult::CONSUMED;
		}

		std::string nonce = nn::GenerateNonce();

		if(nonce.empty()) {
			Log::Error("Failed to generate nonce");
		}

		// for now accept everything
		Ref<nn::NetAllocator> alloc = service->MakeAllocator(1024);
		np::Control * localControl = alloc->MakeProto<np::Control>();
		localControl->set_sequence(1);
		localControl->set_type(np::MessageType::CONNECT_RESPONSE);
		np::ConnectResponse * connResp = localControl->mutable_connect_response();
		connResp->set_type(connReq->type());
		connResp->set_current_map("mat_test_map");
		connResp->set_nonce(std::move(nonce));
		connResp->set_error_code(0);

		static int32_t idGen = 1;

		Ref<Connection> conn = std::make_shared<Connection>(service->GetIOContext());
		conn->id = idGen++;
		conn->dtlsRoute = route;
		conn->pmtu = nn::MtuValue{ route->mtu };
		conn->endpoint = route->endpoint;
		conn->remoteGameSequence = 0;
		conn->localControlSequence = 1;
		conn->localGameSequence = 1;
		conn->remoteControlSequence = peerControl->sequence();
		conn->state = nn::ConnectionState::SYNCHRONIZING;
		conn->tickInterval = std::chrono::milliseconds(Netcode::Config::Get<uint32_t>(L"network.client.tickIntervalMs:u32"));
		
		connResp->set_player_id(conn->id);
		
		nn::ControlMessage localCm;
		localCm.control = localControl;
		localCm.allocator = alloc;

		server->OnPlayerConnected(conn.get());
		connections->AddConnection(conn);

		service->Send(alloc, alloc->MakeCompletionToken<nn::TrResult>(), conn->dtlsRoute, localCm, conn->endpoint, conn->pmtu, nn::ResendArgs{ 1000, 3 });

		return nn::FilterResult::CONSUMED;
	}
};

class ServerClockSyncRequestFilter : public nn::FilterBase {
	Netcode::Timestamp startedAt;
	Netcode::GameClock * clock;
	GameServer * server;
	Connection * connection;
	bool isDone;

	nn::FilterResult HandleConnectDone(np::Control* ctrl) {
		if(isDone)
			return nn::FilterResult::IGNORED;
		
		if(!ctrl->has_connect_done())
			return nn::FilterResult::IGNORED;

		const Netcode::Duration rtt{ ctrl->connect_done().measured_rtt() };

		connection->rtt = rtt;
		connection->state = nn::ConnectionState::ESTABLISHED;
		isDone = true;
		server->OnPlayerJoined(connection);

		return nn::FilterResult::CONSUMED;
	}
	
public:
	ServerClockSyncRequestFilter(GameServer * srv, Connection * conn) :
		clock{ &srv->gameClock }, server{ srv }, connection{ conn }, isDone{false} {
		startedAt = Netcode::SystemClock::LocalNow();
	}

	bool CheckTimeout(Netcode::Timestamp checkAt) override {
		return (checkAt - startedAt) > std::chrono::seconds(10);
	}

	bool IsCompleted() const override {
		return isDone;
	}
	
	nn::FilterResult Run(Ptr<nn::NetcodeService> service, Ptr<nn::DtlsRoute> route, nn::ControlMessage & cm) override {
		np::Control * ctrl = cm.control;

		if(ctrl->type() == np::MessageType::CONNECT_DONE) {
			return HandleConnectDone(ctrl);
		}
		
		if(ctrl->type() != np::MessageType::CLOCK_SYNC_REQUEST ||
		   !ctrl->has_time_sync()) {
			return nn::FilterResult::IGNORED;
		}

		if(route == nullptr)
			return nn::FilterResult::IGNORED;
		
		Ref<nn::ConnectionBase> conn = service->GetConnections()->GetConnectionByEndpointUnsafe(route->endpoint);

		if(conn == nullptr)
			return nn::FilterResult::IGNORED;

		ctrl->set_type(np::MessageType::CLOCK_SYNC_RESPONSE);
		ctrl->set_sequence(conn->localControlSequence++);

		
		np::TimeSync* timeSync = ctrl->mutable_time_sync();
		timeSync->set_server_req_reception(Netcode::ConvertTimestampToUInt64(cm.packet->GetTimestamp() - clock->GetEpoch()));
		timeSync->set_server_resp_transmission(Netcode::ConvertTimestampToUInt64(Netcode::SystemClock::LocalNow() - clock->GetEpoch()));
		
		cm.packet = nullptr;
		
		service->Send(cm, route);

		return nn::FilterResult::CONSUMED;
	}
};


static bool ConvertAction(ExtClientAction& dst, const np::ActionPrediction& pred) {
	const np::ActionType type = pred.type();

	if(type == np::ActionType::MOVEMENT) {
		if(!pred.has_action_position()) {
			return false;
		}

		dst.timestamp = Netcode::ConvertUInt64ToTimestamp(pred.timestamp());
		dst.movementActionData.position = ConvertFloat3(pred.action_position());
		dst.id = pred.id();
		dst.type = ActionType::MOVEMENT;
		return true;
	}

	if(type == np::ActionType::SPAWN) {
		dst.timestamp = Netcode::ConvertUInt64ToTimestamp(pred.timestamp());
		dst.movementActionData.position = Netcode::Float3::Zero;
		dst.id = pred.id();
		dst.type = ActionType::SPAWN;
		return true;
	}

	if(type == np::ActionType::FIRE) {
		if(!pred.has_action_position() || !pred.has_action_delta()) {
			return false;
		}

		dst.timestamp = Netcode::ConvertUInt64ToTimestamp(pred.timestamp());
		dst.fireActionData.position = ConvertFloat3(pred.action_position());
		dst.fireActionData.direction = ConvertFloat3(pred.action_delta());
		dst.id = pred.id();
		dst.type = ActionType::FIRE;
		return true;
	}
	
	return false;
}

void GameServer::OnPlayerConnected(Connection* connection) {
	
	GameObject* gameObj = CreateRemoteAvatar(connection->tickInterval, controllerManager.Get());
	Network* network = gameObj->GetComponent<Network>();
	network->replDesc = CreateRemoteAvatarReplDesc();
	network->owner = connection->id;

	std::ostringstream oss;
	oss << "p:" << connection->id;
	gameObj->name = oss.str();
	RemotePlayerScript* rps = gameObj->GetComponent<Script>()->GetScript<RemotePlayerScript>(0);
	physx::PxActor* actor = rps->GetController()->getActor();

	Netcode::UndefinedBehaviourAssertion(actor->getScene() == pxScene);
	
	pxScene->removeActor(*actor);
	rps->serverHistory = std::make_unique<HistoryBuffer<ServerFrameData>>(128);

	
	connection->gameObject = gameObj;
	connection->remotePlayerScript = rps;
	connection->filters.emplace_back(std::make_unique<ServerClockSyncRequestFilter>(this, connection));
}

void GameServer::OnPlayerJoined(Connection * connection) {
	PlayerStatEntry pse;
	pse.name = "TestName";
	pse.kills = 0;
	pse.deaths = 0;
	pse.id = connection->id;
	pse.ping = static_cast<int32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(connection->rtt).count());

	scoreboard->stats.push_back(pse);

	Log::Debug("PlayerJoined");
	
	connections->ForeachUnsafe<Connection>([&](Connection * existingConn) -> void {
		Network * nc = existingConn->gameObject->GetComponent<Network>();

		const ServerCommand cmd = ServerCommand::CreateObject(existingConn->id, REPL_TYPE_REMOTE_AVATAR, nc->id);
		connection->redundancyBuffer.Add(connection->localGameSequence,
			ServerReconciliation::CreateCommand(connection->localCommandIndex++, cmd));
	});
	
	Network * networkComponent = scoreboardObject->GetComponent<Network>();
	const ServerCommand spawnCmd = ServerCommand::CreateObject(0, REPL_TYPE_SCOREBOARD, networkComponent->id);
	const ServerReconciliation sr = ServerReconciliation::CreateCommand(connection->localCommandIndex++, spawnCmd);
	connection->redundancyBuffer.Add(connection->localGameSequence, sr);
}

void GameServer::FetchActions() {
	Netcode::Stopwatch sw;
	sw.Start();
	
	connections->ForeachUnsafe<Connection>([&](Connection * conn) -> void {
		nn::Node<nn::GameMessage> * nodes = conn->sharedQueue.ConsumeAll();

		if(nodes == nullptr)
			return;

		for(NodeIter<nn::GameMessage> it = nodes; it != nullptr; it++) {
			Netcode::Stopwatch perfUpdateSw;
			perfUpdateSw.Start();
			np::ClientUpdate * update = ParseClientUpdate(it.operator->());
			perfUpdateSw.Stop();

			perfCurrent.parseTime += perfUpdateSw.GetElapsedDuration();
			
			if(update == nullptr)
				continue;

			const uint32_t firstActionIndex = conn->remoteActionIndex;
			uint32_t maxActionIndex = conn->remoteActionIndex;
			for(const np::ActionPrediction & pred : update->predictions()) {
				ExtClientAction ca;

				if(!ConvertAction(ca, pred))
					continue;

				/*this assumes the predictions are in order*/
				if(maxActionIndex >= ca.id) {
					if(ca.id > firstActionIndex) {
						Log::Debug("dropping action because it was a duplicate or in invalid order");
					}
					continue;
				}

				maxActionIndex = std::max(maxActionIndex, ca.id);
				
				ca.owner = conn;
				actions.emplace_back(ca);
				conn->remoteActionIndex = maxActionIndex;
			}

			for(const np::ReplData& rd : update->replications()) {
				Network * network = conn->gameObject->GetComponent<Network>();
				if(network->id == rd.object_id()) {
					ReplicateRead(conn->gameObject, network, rd.data(), &gameClock, 0, ActorType::SERVER);
				}
			}
			
			conn->redundancyBuffer.Confirm(update->received_id());
			conn->remoteGameSequence = std::max(conn->remoteGameSequence, it->sequence);

			if(maxActionIndex > 0) {
				conn->dtlsRoute->lastReceivedAt = std::max(conn->dtlsRoute->lastReceivedAt, Netcode::SystemClock::LocalNow());
				conn->remoteActionIndex = std::max(conn->remoteActionIndex, maxActionIndex);
			}
		}
	});

	std::sort(std::begin(actions), std::end(actions), [](const ExtClientAction & a, const ExtClientAction & b) -> bool {
		return (a.timestamp < b.timestamp);
	});

	sw.Stop();
	perfCurrent.receiveTime = sw.GetElapsedDuration();
}

void GameServer::HandleMovementAction(const ExtClientAction& action) {
	Connection * connection = action.owner;
	GameObject * gameObject = connection->gameObject;
	RemotePlayerScript * rps = connection->remotePlayerScript;
	Transform * transform = gameObject->GetComponent<Transform>();
	Network * network = gameObject->GetComponent<Network>();

	if(network->state != PlayerState::ALIVE) {
		ServerReconciliation sr = {};
		sr.actionType = action.type;
		sr.id = action.id;
		sr.type = ReconciliationType::REJECTED;
		connection->redundancyBuffer.Add(connection->localGameSequence, sr);
		Log::Debug("Rejecting movement because player is not alive");
		return;
	}

	Netcode::Stopwatch perfMovementSw; perfMovementSw.Start();
	
	const Vector3 position = transform->position;
	const Vector3 predictedPosition = action.movementActionData.position;

	physx::PxController* controller = rps->GetController();

	Vector3 displacement = predictedPosition - position;
	const Netcode::Timestamp currentTimestamp = gameClock.GetGlobalTime();
	Netcode::Duration deltaTime = currentTimestamp - rps->serverLastUpdate;
	const float dt = std::chrono::duration<float>(deltaTime).count();
	rps->serverLastUpdate = currentTimestamp;

	if(deltaTime > Netcode::Duration{}) {
		physx::PxFilterData fd;
		fd.word0 = PHYSX_COLLIDER_TYPE_WORLD | PHYSX_COLLIDER_TYPE_KILLZONE | PHYSX_COLLIDER_TYPE_SERVER_HITBOX;
		fd.word1 = PHYSX_COLLIDER_TYPE_SERVER_HITBOX;
		physx::PxControllerFilters filters;
		filters.mFilterData = &fd;

		controller->move(ToPxVec3(displacement), 0.1f, dt, filters);
	}

	const Vector3 footPos = Netcode::ToFloat3(Netcode::ToPxVec3(controller->getFootPosition()));
	const Vector3 positionDelta = predictedPosition - footPos;

	transform->position = footPos;

	ServerReconciliation sr = {};
	sr.actionType = action.type;
	sr.id = action.id;
	
	// 40 unit diff is fine
	if(positionDelta.LengthSq() > 1600.0f) {
		sr.type = ReconciliationType::REJECTED;
		sr.movementCorrection.position = transform->position;
	} else {
		sr.type = ReconciliationType::ACCEPTED;
	}
	
	connection->redundancyBuffer.Add(connection->localGameSequence, sr);

	perfMovementSw.Stop();

	perfCurrent.movementTime += perfMovementSw.GetElapsedDuration();
}

void GameServer::HandleFireAction(const ExtClientAction & action) {

	Netcode::Stopwatch perfReconSw; perfReconSw.Start();
	Connection * connection = action.owner;
	GameObject * gameObject = connection->gameObject;
	Transform * transform = gameObject->GetComponent<Transform>();
	Network * network = gameObject->GetComponent<Network>();
	physx::PxController * shooterCtrl = connection->remotePlayerScript->GetController();
	
	const Vector3 rayOrigin = action.fireActionData.position;
	const Vector3 dir = action.fireActionData.direction;
	const float dirLen = dir.Length();

	// generous epsilon
	if(dirLen < 0.75f || dirLen > 1.25f) {
		Log::Debug("Dir vector length is invalid");
		return;
	}

	const Vector3 rayDir = dir / dirLen;

	const Netcode::Timestamp globalTime = gameClock.GetGlobalTime();
	const Netcode::Timestamp timestamp = action.timestamp;
	const Netcode::Duration ind = connection->tickInterval.load();
	const Netcode::Duration delta = std::clamp<Netcode::Duration>(timestamp - globalTime, -ind, ind);
	const Netcode::Duration td = std::clamp<Netcode::Duration>(connection->rtt / 2 + ind,
		Netcode::Duration{},
		std::chrono::milliseconds(100));

	ServerReconciliation sr;
	sr.type = ReconciliationType::ACCEPTED;
	sr.actionType = action.type;
	sr.id = action.id;
	connection->redundancyBuffer.Add(connection->localGameSequence, sr);

	// might be in an invalid state
	if(network->state != PlayerState::ALIVE)
		return;

	// no need to check if the player is alone
	if(connections->GetConnectionCount() <= 1)
		return;

	Netcode::Timestamp reconstructionTime = globalTime - delta - td;
	pxScene->removeActor(*shooterCtrl->getActor());
	network->state = PlayerState::DISABLED;
	
	connections->ForeachUnsafe<Connection>([&](Connection * conn) -> void {
		if(connection == conn)
			return;

		Network * netw = conn->gameObject->GetComponent<Network>();
		RemotePlayerScript * rps = conn->remotePlayerScript;
		physx::PxController * ctrl = rps->GetController();

		if(netw->state != PlayerState::ALIVE && netw->state != PlayerState::DISABLED)
			return;

		Netcode::Stopwatch perfPosCalcSw; perfPosCalcSw.Start();

		auto [p1, p0] = rps->serverHistory->FindAt(reconstructionTime);

		if((p0 == nullptr || p0->value.state == PlayerState::DEAD) || (p1 == nullptr || p1->value.state == PlayerState::DEAD)) {
			netw->state = PlayerState::DISABLED;
			Netcode::Stopwatch perfManipSw; perfManipSw.Start();
			pxScene->removeActor(*ctrl->getActor());
			perfManipSw.Stop();
			perfCurrent.numPxManip++;
			perfCurrent.pxSceneManipTime += perfManipSw.GetElapsedDuration();
			return;
		}

		if(netw->state == PlayerState::DISABLED) {
			netw->state = PlayerState::ALIVE;
			Netcode::Stopwatch perfManipSw; perfManipSw.Start();
			pxScene->addActor(*ctrl->getActor());
			perfManipSw.Stop();
			perfCurrent.numPxManip++;
			perfCurrent.pxSceneManipTime += perfManipSw.GetElapsedDuration();
			return;
		}

		Netcode::Timestamp t1 = p1->timestamp;
		Netcode::Timestamp t0 = p0->timestamp;

		Netcode::Duration frameDuration = t1 - t0;
		Netcode::Duration relativeDuration = reconstructionTime - t0;

		const float lerpArg = std::clamp(std::chrono::duration<float>(relativeDuration).count() /
			std::chrono::duration<float>(frameDuration).count(),
			0.0f,
			1.0f);

		const Float3 reconstructedPos = Vector3::Lerp(p0->value.position, p1->value.position, lerpArg);

		perfPosCalcSw.Stop();
	
		perfCurrent.numPosCalc++;
		perfCurrent.posCalcTime += perfPosCalcSw.GetElapsedDuration();

		Netcode::Stopwatch perfPoseSw; perfPoseSw.Start();
		ctrl->setFootPosition(Netcode::ToPxExtVec3(reconstructedPos));
		perfPoseSw.Stop();

		perfCurrent.numPxPose++;
		perfCurrent.pxScenePoseTime += perfPoseSw.GetElapsedDuration();
	});

	physx::PxRaycastBuffer hit;
	physx::PxQueryFilterData fd;
	fd.flags |= physx::PxQueryFlag::eANY_HIT;
	fd.data.word0 = PHYSX_COLLIDER_TYPE_LOCAL_HITBOX;
	fd.data.word1 = PHYSX_COLLIDER_TYPE_WORLD | PHYSX_COLLIDER_TYPE_SERVER_HITBOX;
	fd.data.word2 = 0;
	fd.data.word3 = 0;

	if(!pxScene->raycast(ToPxVec3(rayOrigin), ToPxVec3(rayDir), 10000.0f, hit, physx::PxHitFlag::eDEFAULT, fd)) {
		//Log::Debug("SRV: missed everything");
	} else {
		if(hit.hasBlock) {
			physx::PxFilterData hitFilter = hit.block.shape->getQueryFilterData();

			if(hitFilter.word0 & PHYSX_COLLIDER_TYPE_WORLD) {
				//Log::Debug("SRV: environment hit");
			} else {
				//Log::Debug("SRV: player hit");
			}
		}
	}

	/* restore physx scene */
	pxScene->addActor(*shooterCtrl->getActor());
	

	connections->ForeachUnsafe<Connection>([&](Connection * conn) -> void {
		Network * netw = conn->gameObject->GetComponent<Network>();
		Transform * tr = conn->gameObject->GetComponent<Transform>();
		physx::PxController * remCtrl = conn->remotePlayerScript->GetController();

		if(netw->state == PlayerState::DISABLED) {
			netw->state = PlayerState::ALIVE;
			perfCurrent.numPxManip++;
			Netcode::Stopwatch addSw; addSw.Start();
			pxScene->addActor(*remCtrl->getActor());
			addSw.Stop();
			perfCurrent.pxSceneManipTime += addSw.GetElapsedDuration();
		}

		perfCurrent.numPxPose++;
		Netcode::Stopwatch manipSw; manipSw.Start();
		remCtrl->setFootPosition(ToPxExtVec3(tr->position));
		manipSw.Stop();
		perfCurrent.pxScenePoseTime += manipSw.GetElapsedDuration();
	});

	perfReconSw.Stop();
	perfCurrent.numShots++;
	perfCurrent.reconstrTime += perfReconSw.GetElapsedDuration();
}

void GameServer::HandleSpawnAction(const ExtClientAction& action) {
	Connection * connection = action.owner;
	GameObject * gameObject = connection->gameObject;
	RemotePlayerScript * rps = connection->remotePlayerScript;
	Transform * transform = gameObject->GetComponent<Transform>();
	Network * networkComponent = gameObject->GetComponent<Network>();
	Camera * camera = gameObject->GetComponent<Camera>();

	ServerReconciliation sr = {};
	sr.id = action.id;
	sr.actionType = action.type;
	sr.type = ReconciliationType::REJECTED;
	
	if(networkComponent->state != PlayerState::ALIVE) {
		networkComponent->state = PlayerState::ALIVE;
		sr.type = ReconciliationType::ACCEPTED;
		pxScene->addActor(*rps->GetController()->getActor());
	}
	
	connection->redundancyBuffer.Add(connection->localGameSequence, sr);

	if(sr.type == ReconciliationType::ACCEPTED) {
		ServerReconciliation csr = {};
		csr.id = 0;
		csr.type = ReconciliationType::COMMAND;
		csr.actionType = ActionType::NOOP;
		csr.command.type = CommandType::CREATE_OBJECT;
		csr.command.objectId = nextGameObjectId++;
		csr.command.objectType = REPL_TYPE_REMOTE_AVATAR;
		csr.command.subject = connection->id;
		networkComponent->id = csr.command.objectId;
		rps->serverLastUpdate = gameClock.GetGlobalTime();

		int value = (int)connections->GetConnectionCount();

		const SpawnPoint sp = spawnPoints[value];
		transform->position = sp.position;
		camera->ahead = Vector3{ Float3::UnitZ }.Rotate(sp.rotation);
		rps->GetController()->setFootPosition(ToPxExtVec3(transform->position));
		csr.replData = ReplicateWrite(gameObject, networkComponent);

		connections->ForeachUnsafe<Connection>([&](Connection * conn) -> void {
			csr.id = conn->localCommandIndex++;
			Log::Debug("Sending spawn to: {0}", conn->id);
			conn->redundancyBuffer.Add(conn->localGameSequence, csr);
		});
	}
}

void GameServer::DisconnectPlayer(Connection * connection) {
	Ref<nn::ConnectionBase> lifetime = connection->shared_from_this();
	
	connections->RemoveConnection(lifetime);

	pxScene->removeActor(*connection->remotePlayerScript->GetController()->getActor());
	gameScene->RemoveWithHierarchy(connection->gameObject);
	connection->gameObject = nullptr;
	connection->remotePlayerScript = nullptr;
	connection->dtlsRoute->state = Netcode::Network::DtlsRouteState::DISCONNECTED;
}

void GameServer::ProcessActions() {
	Netcode::Stopwatch sw;
	sw.Start();
	Netcode::Timestamp serverTimestamp = gameClock.GetGlobalTime();
	
	for(const ExtClientAction& action : actions) {
		const Netcode::Duration timestampDelta = serverTimestamp - action.timestamp;

		/*
		 * client is predicting ahead by RTT/2, that means if for some reason the message
		 * arrives instantly, then the delta can only be at most RTT/2 otherwise the client
		 * did not report its RTT properly.
		 *
		 * The usual delta should be around 0 as it takes the message RTT/2 time to arrive.
		 *
		 * if the timestamp delta is less than -historyDuration (usually 1 second), the action
		 * gets rejected. Movement actions can recover if we have more recent data than the timestamp.

		if(timestampDelta > action.owner->rtt / 2) {
			// reject
			continue;
		}

		// TODO: timestampDelta < historyDuration
		if(timestampDelta < std::chrono::seconds(1)) {
			// any action this old are silently dropped
			continue;
		}
		 */

		if(action.id > action.owner->remoteActionIndex) {
			
		}
		
		switch(action.type) {
			case ActionType::MOVEMENT: HandleMovementAction(action); break;
			case ActionType::FIRE: HandleFireAction(action); break;
			case ActionType::SPAWN: HandleSpawnAction(action); break;
			default: break;
		}
	}

	actions.clear();

	sw.Stop();

	perfCurrent.processTime = sw.GetElapsedDuration();
}

void GameServer::FetchControlMessages() {

}

void GameServer::ProcessControlMessages() {
	Netcode::Timestamp timestamp = Netcode::SystemClock::LocalNow();
	
	connections->ForeachUnsafe<Connection>([&](Connection * conn) -> void {
		nn::Node<nn::ControlMessage> * node = conn->sharedControlQueue.ConsumeAll();
		
		for(NodeIter<nn::ControlMessage> it = node; it != nullptr; it++) {
			service->ApplyFilters(conn->filters, conn->dtlsRoute, *it.operator->());
		}

		service->CheckFilterCompletion(conn->filters);
	});
}

void GameServer::SaveState() {
	Netcode::Timestamp serverTime = gameClock.GetGlobalTime();

	connections->ForeachUnsafe<Connection>([&](Connection * conn) -> void {
		GameObject* obj = conn->gameObject;
		Transform * transform = obj->GetComponent<Transform>();
		Network * netw = obj->GetComponent<Network>();
		conn->remotePlayerScript->serverHistory->Insert(serverTime,
			ServerFrameData{ transform->position, netw->state });
	});
}

void GameServer::CheckTimeouts() {
	Netcode::Timestamp timestamp = Netcode::SystemClock::LocalNow();

	std::vector<Connection *> timeouts;
	
	connections->ForeachUnsafe<Connection>([&](Connection * conn) -> void {
		if((timestamp - conn->dtlsRoute->lastReceivedAt) > std::chrono::seconds(5)) {
			Log::Debug("Server: timeout {0}", conn->id);
			// drop connection, broadcast event
			conn->state = nn::ConnectionState::TIMEDOUT;
			timeouts.push_back(conn);
		}
	});
	
	if(timeouts.empty())
		return;

	connections->ForeachUnsafe<Connection>([&](Connection * conn) ->void {
		if(conn->state != nn::ConnectionState::TIMEDOUT) {
			for(Connection* c : timeouts) {
				ServerReconciliation sr = {};
				sr.type = ReconciliationType::COMMAND;
				sr.id = conn->localCommandIndex++;
				Log::Debug("Sending timeout command: {0}", (int)sr.id);
				sr.actionType = ActionType::NOOP;
				sr.command.subject = c->id;
				sr.command.type = CommandType::PLAYER_TIMEDOUT;
				conn->redundancyBuffer.Add(conn->localGameSequence, sr);
			}
		}
	});

	for(Connection* c : timeouts) {
		DisconnectPlayer(c);
	}
}

static void AddActionResult(np::ServerUpdate* su, const RedundancyItem& item) {
	const ServerReconciliation & recon = std::get<ServerReconciliation>(item.storage);
	
	if(recon.type != ReconciliationType::COMMAND) {
		np::ActionResult * ar = su->add_action_results();

		np::ActionResultType result = (recon.type == ReconciliationType::ACCEPTED) ?
			np::ActionResultType::ACCEPTED : np::ActionResultType::REJECTED;
		
		ar->set_type(static_cast<np::ActionType>(recon.actionType));
		ar->set_result(result);
		ar->set_id(recon.id);

		if(recon.actionType == ActionType::MOVEMENT &&
			recon.type == ReconciliationType::REJECTED) {
			ConvertFloat3(ar->mutable_action_position(), recon.movementCorrection.position);
		}

		if(recon.actionType == ActionType::SPAWN &&
			recon.type == ReconciliationType::ACCEPTED) {
			ConvertFloat3(ar->mutable_action_position(), recon.movementCorrection.position);
		}
	}

	if(recon.type == ReconciliationType::COMMAND) {
		np::ServerCommand * command = su->add_commands();
		command->set_id(recon.id);
		command->set_type(static_cast<np::CommandType>(recon.command.type));
		command->set_subject(recon.command.subject);
		command->set_object_id(recon.command.objectId);
		command->set_object_type(recon.command.objectType);
		command->set_repl_data(recon.replData);
	}
}

static void ReplicateGameObject(np::ServerUpdate* su, GameObject* obj) {
	Network * network = obj->GetComponent<Network>();

	std::string binary = ReplicateWrite(obj, network);

	if(binary.empty())
		return;

	np::ReplData * replData = su->add_replications();
	replData->set_object_id(network->id);
	replData->set_data(std::move(binary));
}

void GameServer::BuildServerUpdates() {
	
	connections->ForeachUnsafe<Connection>([&](Connection * conn) -> void {
		const uint32_t sequence = conn->localGameSequence++;

		Ref<nn::NetAllocator> allocator = service->MakeAllocator(2048);
		np::ServerUpdate* su = allocator->MakeProto<np::ServerUpdate>();

		su->set_received_id(conn->remoteGameSequence);
		
		for(const RedundancyItem& item : conn->redundancyBuffer.GetBuffer()) {
			AddActionResult(su, item);
		}

		connections->ForeachUnsafe<Connection>([&](Connection * inner) -> void {
			ReplicateGameObject(su, inner->gameObject);
		});

		if(scoreboardReplInterval < Netcode::Duration{}) {
			if(!scoreboard->stats.empty()) {
				ReplicateGameObject(su, scoreboardObject);
				scoreboardReplInterval = std::chrono::seconds(1);
			}
		}
		
		conn->serverUpdate = su;
		conn->message.allocator = std::move(allocator);
		conn->message.sequence = sequence;
	});
}

void GameServer::SendServerUpdates() {
	connections->ForeachUnsafe<Connection>([&](Connection * conn) -> void {
		size_t contentSize = conn->serverUpdate->ByteSizeLong();
		uint8_t * content = conn->message.allocator->MakeArray<uint8_t>(contentSize);

		if(conn->serverUpdate->SerializeToArray(content, static_cast<int32_t>(contentSize))) {
			conn->message.content = Netcode::ArrayView<uint8_t>{ content, contentSize };
		}
		
		service->Send(conn->message, conn);

		conn->message.allocator.reset();
		conn->message.sequence = 0;
		conn->message.content = Netcode::ArrayView<uint8_t>{};
	});
}

GameServer::GameServer() : serverSession{}, actions{}, service{}, connections{}, gameClock{}, nextGameObjectId{ 1 } {
}

void GameServer::Tick() {
	gameClock.Tick();
	Netcode::Duration dt = gameClock.GetDeltaTime();
	scoreboardReplInterval -= dt;

	perfCurrent.timestamp = gameClock.GetGlobalTime();

	Netcode::Stopwatch sw;
	
	sw.Start();
	
	FetchActions();

	ProcessActions();

	SaveState();

	FetchControlMessages();

	ProcessControlMessages();

	CheckTimeouts();

	BuildServerUpdates();

	SendServerUpdates();

	service->RunFilters();

	sw.Stop();

	static bool written = false;

	perfCurrent.frameTime = sw.GetElapsedDuration();
	if((gameClock.GetLocalTime() - Netcode::Timestamp{}) > std::chrono::seconds(10)) {
		if(!written) {
			perf.emplace_back(perfCurrent);
		}
	}

	perfCurrent = PerfData{};
	if(!written && (gameClock.GetLocalTime() - Netcode::Timestamp{} > std::chrono::seconds(130))) {
		std::ofstream ofs{ "perf.csv" };
		
		const uint32_t intervalMs = Netcode::Config::GetOptional<uint32_t>(L"network.client.tickIntervalMs:u32", 250u);

		ofs << R"("timestamp","players","interval[ms]","frametime[ns]","recv[ns]","parse[ns]","proc[ns]","move[ns]","reconst[ns]",)";
		ofs << R"("numPosCalc","sumPosCalc[ns]","numPxManip","sumPxManip[ns]","numPxPose","sumPxPose[ns]")" << std::endl;
		
		for(const PerfData& p : perf) {
			ofs << std::chrono::duration<double>(p.timestamp - Netcode::Timestamp{}).count() << ",";
			ofs << connections->GetConnectionCount() << ",";
			ofs << std::chrono::milliseconds{ intervalMs }.count() << ",";
			ofs << p.frameTime.count() << ",";
			ofs << p.receiveTime.count() << ",";
			ofs << p.parseTime.count() << ",";
			ofs << p.processTime.count() << ",";
			ofs << p.movementTime.count() << ",";
			ofs << p.reconstrTime.count() << ",";
			ofs << p.numPosCalc << ",";
			ofs << p.posCalcTime.count() << ",";
			ofs << p.numPxManip << ",";
			ofs << p.pxSceneManipTime.count() << ",";
			ofs << p.numPxPose << ",";
			ofs << p.pxScenePoseTime.count() << std::endl;
		}

		ofs.close();

		Log::Debug("CSV saved");
		written = true;
	}
}

void GameServer::Start(Netcode::Module::INetworkModule* network) {
	gameClock.SetEpoch(Netcode::SystemClock::LocalNow() - Netcode::Timestamp{});

	perf.reserve(16384);
	
	serverSession = std::dynamic_pointer_cast<nn::ServerSession>(network->CreateServer());
	serverSession->Start();
	
	service = serverSession->GetService();
	service->AddFilter(std::make_unique<ServerConnRequestFilter>(this));
	
	connections = service->GetConnections();

	scoreboardObject = CreateScoreboard(nextGameObjectId++);
	scoreboard = scoreboardObject->GetComponent<Script>()->GetScript<ScoreboardScript>(0);
	scoreboardReplInterval = std::chrono::seconds(1);

	gameScene = Service::Get<GameSceneManager>()->GetScene();
	pxScene = gameScene->GetPhysXScene();
	controllerManager = PxCreateControllerManager(*pxScene);
	
	/*
	 * For now initialize spawn points here
	 */
	spawnPoints = {
		SpawnPoint{ Float3{ 1000.0f, 200.0f, 0.0f }, Quaternion{} },
		SpawnPoint{ Float3{ 500.0f, 200.0f, 0.0f }, Quaternion{} },
		SpawnPoint{ Float3{ 0.0f, 200.0f, 0.0f }, Quaternion{} },
		SpawnPoint{ Float3{ -500.0f, 200.0f, 0.0f }, Quaternion{} },
		SpawnPoint{ Float3{ -1000.0f, 200.0f, 0.0f }, Quaternion{} },
		SpawnPoint{ Float3{ -1500.0f, 200.0f, 0.0f }, Quaternion{} },
		SpawnPoint{ Float3{ -2000.0f, 200.0f, 0.0f }, Quaternion{} },
	};

	if(spawnPoints.empty()) {
		spawnPoints.push_back(SpawnPoint{ Float3::Zero, Quaternion{} });
	}
	
	uniformIntDistribution = std::uniform_int_distribution<int>{ 0, static_cast<int>(spawnPoints.size() - 1) };
}
