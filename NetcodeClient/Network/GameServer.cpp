#include "GameServer.h"
#include "../Services.h"
#include "../GameScene.h"
#include <Netcode/Network/Service.h>
#include <sstream>

class ServerConnRequestFilter : public nn::FilterBase {

public:
	nn::FilterResult Run(Ptr<nn::NetcodeService> service, Ptr<nn::DtlsRoute> route, Netcode::Timestamp timestamp, nn::ControlMessage & cm) override {
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
		conn->state = nn::ConnectionState::ESTABLISHED;
		conn->gameObject = CreateRemoteAvatar();

		connections->ForeachUnsafe<Connection>([&](Connection * existingConn) -> void {
			Network * nc = existingConn->gameObject->GetComponent<Network>();
			ServerReconciliation sr;
			sr.type = ReconciliationType::COMMAND;
			sr.actionType = ActionType::NOOP;
			sr.id = conn->localCommandIndex++;
			sr.command.type = CommandType::CREATE_OBJECT;
			sr.command.subject = existingConn->id;
			sr.command.objectId = nc->id;
			sr.command.objectType = REPL_TYPE_REMOTE_AVATAR;
			conn->redundancyBuffer.Add(conn->localGameSequence, sr);
		});
		
		connResp->set_player_id(conn->id);

		std::ostringstream oss;
		oss << "p:" << conn->id;
		conn->gameObject->name = oss.str();
		conn->remotePlayerScript =
			dynamic_cast<RemotePlayerScript *>(conn->gameObject->GetComponent<Script>()->GetScript(0));
		
		nn::ControlMessage localCm;
		localCm.control = localControl;
		localCm.allocator = alloc;

		connections->AddConnection(conn);

		service->Send(alloc, alloc->MakeCompletionToken<nn::TrResult>(), conn->dtlsRoute, localCm, conn->endpoint, conn->pmtu, nn::ResendArgs{ 1000, 3 });

		return nn::FilterResult::CONSUMED;
	}
};

class ServerClockSyncRequestFilter : public nn::FilterBase {
public:
	nn::FilterResult Run(Ptr<nn::NetcodeService> service, Ptr<nn::DtlsRoute> route, Netcode::Timestamp timestamp, nn::ControlMessage & cm) override {
		np::Control * ctrl = cm.control;
		
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
		
		np::TimeSync* timeSync = ctrl->mutable_time_sync();
		timeSync->set_server_req_reception(Netcode::ConvertTimestampToUInt64(cm.packet->GetTimestamp()));
		timeSync->set_server_resp_transmission(Netcode::ConvertTimestampToUInt64(Netcode::SystemClock::LocalNow()));

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

	if(type == np::ActionType::LOOK) {
		if(!pred.has_action_position()) {
			return false;
		}
		dst.timestamp = Netcode::ConvertUInt64ToTimestamp(pred.timestamp());
		dst.id = pred.id();
		dst.type = ActionType::LOOK;
		const Netcode::Float3 f3 = ConvertFloat3(pred.action_position());
		dst.lookActionData.pitch = f3.x;
		dst.lookActionData.yaw = f3.y;
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

void GameServer::FetchActions() {
	connections->ForeachUnsafe<Connection>([&](Connection * conn) -> void {
		nn::Node<nn::GameMessage> * node = conn->sharedQueue.ConsumeAll();

		if(node == nullptr)
			return;

		uint32_t maxActionIndex = 0;

		for(NodeIter<nn::GameMessage> it = node; it != nullptr; ++it) {
			np::ClientUpdate * update = ParseClientUpdate(it.operator->());

			if(update == nullptr)
				continue;

			for(const np::ActionPrediction & pred : update->predictions()) {
				ExtClientAction ca;

				if(!ConvertAction(ca, pred))
					continue;

				if(conn->remoteActionIndex >= ca.id)
					continue;

				maxActionIndex = std::max(maxActionIndex, ca.id);
				
				ca.owner = conn;
				actions.emplace_back(ca);
			}

			for(const np::ReplData& rd : update->replications()) {
				Network * network = conn->gameObject->GetComponent<Network>();
				if(network->id == rd.object_id()) {
					ReplicateRead(conn->gameObject, network, rd.data(), &gameClock, 0, ActorType::SERVER);
				}
			}
			
			conn->redundancyBuffer.Confirm(update->received_id());
			conn->remoteGameSequence = std::max(conn->remoteGameSequence, node->sequence);
		}

		if(maxActionIndex > 0) {
			conn->dtlsRoute->lastReceivedAt = std::max(conn->dtlsRoute->lastReceivedAt, Netcode::SystemClock::LocalNow());
			conn->remoteActionIndex = std::max(conn->remoteActionIndex, maxActionIndex);
		}
	});

	std::sort(std::begin(actions), std::end(actions), [](const ExtClientAction & a, const ExtClientAction & b) -> bool {
		return a.timestamp < b.timestamp;
	});
}

using Netcode::Vector3;

void GameServer::HandleMovementAction(const ExtClientAction& action) {
	Connection * connection = action.owner;
	GameObject * gameObject = connection->gameObject;
	Transform * transform = gameObject->GetComponent<Transform>();

	const Vector3 position = transform->position;
	const Vector3 predictedPosition = action.movementActionData.position;

	// TODO calculate delta and delta time to compare

	// always accept for now
	transform->position = predictedPosition;

	ServerReconciliation sr = {};
	sr.type = ReconciliationType::ACCEPTED;
	sr.actionType = action.type;
	sr.id = action.id;
	
	connection->redundancyBuffer.Add(connection->localGameSequence, sr);
}

void GameServer::HandleLookAction(const ExtClientAction & action) {
	Connection * connection = action.owner;
	RemotePlayerScript * rps = connection->remotePlayerScript;
}

void GameServer::HandleFireAction(const ExtClientAction& action) {
	Connection * connection = action.owner;
	GameObject * gameObject = connection->gameObject;
	Transform * transform = gameObject->GetComponent<Transform>();

	const Vector3 source = action.fireActionData.position;
	// TODO check if source is plausible

	const Netcode::Timestamp timestamp = action.timestamp;
	// theta, delta is needed.
	
	// its rewind time!
}

void GameServer::HandleSpawnAction(const ExtClientAction& action) {
	Connection * connection = action.owner;
	GameObject * gameObject = connection->gameObject;
	Network * networkComponent = gameObject->GetComponent<Network>();

	ServerReconciliation sr = {};
	sr.id = action.id;
	sr.actionType = action.type;
	sr.type = ReconciliationType::REJECTED;
	
	if(networkComponent->state != PlayerState::ALIVE) {
		networkComponent->state = PlayerState::ALIVE;
		sr.type = ReconciliationType::ACCEPTED;
	}
	
	connection->redundancyBuffer.Add(connection->localGameSequence, sr);

	if(sr.type == ReconciliationType::ACCEPTED) {
		ServerReconciliation csr = {};
		csr.id = 0;
		csr.type = ReconciliationType::COMMAND;
		csr.actionType = ActionType::NOOP;
		csr.command.type = CommandType::CREATE_OBJECT;
		csr.command.objectId = this->nextGameObjectId++;
		csr.command.subject = connection->id;
		networkComponent->id = csr.command.objectId;

		connections->ForeachUnsafe<Connection>([&](Connection * conn) -> void {
			csr.id = conn->localCommandIndex++;
			conn->redundancyBuffer.Add(conn->localGameSequence, csr);
		});
	}
}

void GameServer::DisconnectPlayer(Connection * connection) {
	Ref<nn::ConnectionBase> lifetime = connection->shared_from_this();
	
	connections->RemoveConnection(lifetime);

	GameSceneManager * gsm = Service::Get<GameSceneManager>();
	gsm->GetScene()->RemoveWithHierarchy(connection->gameObject);
	connection->gameObject = nullptr;
	connection->remotePlayerScript = nullptr;
	connection->dtlsRoute->state = Netcode::Network::DtlsRouteState::DISCONNECTED;
}

void GameServer::ProcessActions() {
	for(const ExtClientAction& action : actions) {
		switch(action.type) {
			case ActionType::MOVEMENT: HandleMovementAction(action); break;
			case ActionType::FIRE: HandleFireAction(action); break;
			case ActionType::SPAWN: HandleSpawnAction(action); break;
			case ActionType::LOOK: HandleLookAction(action); break;
			default: break;
		}
	}

	actions.clear();
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
	if(item.reconciliation.type != ReconciliationType::COMMAND) {
		np::ActionResult * ar = su->add_action_results();

		np::ActionResultType result = (item.reconciliation.type == ReconciliationType::ACCEPTED) ?
			np::ActionResultType::ACCEPTED : np::ActionResultType::REJECTED;
		
		ar->set_type(static_cast<np::ActionType>(item.reconciliation.actionType));
		ar->set_result(result);
		ar->set_id(item.reconciliation.id);

		if(item.reconciliation.actionType == ActionType::MOVEMENT &&
		   item.reconciliation.type == ReconciliationType::REJECTED) {
			ConvertFloat3(ar->mutable_action_position(), item.reconciliation.movementCorrection.position);
		}
	}

	if(item.reconciliation.type == ReconciliationType::COMMAND) {
		np::ServerCommand * command = su->add_commands();
		command->set_id(item.reconciliation.id);
		command->set_type(static_cast<np::CommandType>(item.reconciliation.command.type));
		command->set_subject(item.reconciliation.command.subject);
		command->set_object_id(item.reconciliation.command.objectId);
		command->set_object_type(item.reconciliation.command.objectType);
	}
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
			Network* network = inner->gameObject->GetComponent<Network>();

			std::string binary = ReplicateWrite(inner->gameObject, network);

			if(binary.empty())
				return;
			
			np::ReplData* replData = su->add_replications();
			replData->set_object_id(network->id);
			replData->set_data(std::move(binary));
		});
		
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

void GameServer::Tick() {
	
	FetchActions();

	ProcessActions();

	FetchControlMessages();

	ProcessControlMessages();

	CheckTimeouts();

	BuildServerUpdates();

	SendServerUpdates();

	service->RunFilters();
	
}

void GameServer::Start(Netcode::Module::INetworkModule* network) {
	nextGameObjectId = 1;
	gameClock.SetEpoch(Netcode::SystemClock::LocalNow() - Netcode::Timestamp{});
	
	serverSession = std::dynamic_pointer_cast<nn::ServerSession>(network->CreateServer());
	serverSession->Start();
	
	service = serverSession->GetService();
	service->AddFilter(std::make_unique<ServerConnRequestFilter>());
	service->AddFilter(std::make_unique<ServerClockSyncRequestFilter>());
	
	connections = service->GetConnections();
}
