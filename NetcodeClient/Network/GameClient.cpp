#include "GameClient.h"
#include <Netcode/Network/Service.h>
#include "../Services.h"
#include "../Asset/ClientConverters.h"
#include "../AnimationSet.h"
#include <Netcode/MovementController.h>


class ClockSyncResponseFilter : public nn::FilterBase {
	nn::CompletionToken<nn::ClockSyncResult> completionToken;
	Netcode::GameClock * clock;
	Netcode::Timestamp createdAt;
	nn::NtpClockFilter clockFilter;
	uint32_t numUpdates;
public:
	ClockSyncResponseFilter(Netcode::GameClock* clock, nn::CompletionToken<nn::ClockSyncResult> tce) :
		completionToken{ std::move(tce) }, clock{ clock }, createdAt{ Netcode::SystemClock::LocalNow() }, clockFilter{} {
		state = nn::FilterState::RUNNING;
		numUpdates = 0;
	}

	bool CheckTimeout(Netcode::Timestamp checkAt) override {
		if((checkAt - createdAt) > std::chrono::seconds(10)) {
			state = nn::FilterState::COMPLETED;
			nn::ClockSyncResult csr;
			csr.errorCode = make_error_code(Netcode::NetworkErrc::RESPONSE_TIMEOUT);
			csr.delay = 0.0;
			csr.offset = 0.0;
			completionToken->Set(csr);
			return true;
		}
		return false;
	}

	nn::FilterResult Run(Ptr<nn::NetcodeService> service, Ptr<nn::DtlsRoute> route, nn::ControlMessage & cm) override {
		np::Control * control = cm.control;

		if(control->type() != np::MessageType::CLOCK_SYNC_RESPONSE) {
			return nn::FilterResult::IGNORED;
		}

		if(!control->has_time_sync()) {
			return nn::FilterResult::IGNORED;
		}

		np::TimeSync * timeSync = control->mutable_time_sync();
		timeSync->set_client_resp_reception(Netcode::ConvertTimestampToUInt64(cm.packet->GetTimestamp() - clock->GetEpoch()));

		clockFilter.Update(*timeSync);

		numUpdates++;

		if(numUpdates >= 8) {
			nn::ClockSyncResult csr;
			csr.errorCode = make_error_code(Netcode::NetworkErrc::SUCCESS);
			csr.delay = clockFilter.GetDelay();
			csr.offset = clockFilter.GetOffset();
			completionToken->Set(csr);
			state = nn::FilterState::COMPLETED;
		}

		return nn::FilterResult::CONSUMED;
	}
};

static bool ConvertServerReconciliation(ServerReconciliation& dst, const np::ServerCommand& command) {
	ServerReconciliation sr = {};
	sr.type = ReconciliationType::COMMAND;
	sr.actionType = ActionType::NOOP;
	sr.id = command.id();
	sr.replData = command.repl_data();
	sr.command.type = static_cast<CommandType>(command.type());
	sr.command.subject = command.subject();
	sr.command.objectId = command.object_id();
	sr.command.objectType = command.object_type();

	dst = sr;
	return true;
}

static bool ConvertServerReconciliation(ServerReconciliation& dst, const np::ActionResult& result) {
	ServerReconciliation sr = {};
	sr.type = static_cast<ReconciliationType>(result.result());
	sr.actionType = static_cast<ActionType>(result.type());
	sr.id = result.id();

	if(sr.type != ReconciliationType::REJECTED &&
		sr.type != ReconciliationType::ACCEPTED) {
		return false;
	}

	if(sr.type == ReconciliationType::REJECTED && sr.actionType == ActionType::MOVEMENT) {
		if(!result.has_action_position()) {
			return false;
		}
		sr.movementCorrection.position = ConvertFloat3(result.action_position());
	}

	if(sr.type == ReconciliationType::ACCEPTED && sr.actionType == ActionType::SPAWN) {
		if(!result.has_action_position()) {
			return false;
		}
		sr.movementCorrection.position = ConvertFloat3(result.action_position());
	}

	dst = sr;
	return true;
}

GameObject* GameClient::ClientCreateRemoteAvatar(int32_t playerId, uint32_t objId) {
	AssetManager * assetManager = Service::Get<AssetManager>();
	GameScene * scene = Service::Get<GameSceneManager>()->GetScene();
	
	GameObject * avatarCtrl = CreateRemoteAvatar(playerConnection->tickInterval, scene->GetControllerManager());
	GameObject * rifleOrigin = scene->Create("remoteRifleOrigin");
	GameObject * rifle = scene->Create("remoteRifle");
	
	rifleOrigin->AddComponent<Transform>();

	Netcode::Asset::Model * avatarModel = assetManager->Import(L"placeholder_avatar.ncasset");
	Netcode::Asset::Model * rifleModel = assetManager->Import(L"compiled/models/gun_2.ncasset");
	
	auto [rifleTransform, rifleScript] = rifle->AddComponents<Transform, Script>();
	rifleTransform->rotation = Netcode::Quaternion{ 0.0f, -Netcode::C_PIDIV2, 0.0f };
	rifleTransform->position = Netcode::Float3{ 0.0f, -3.5f, 34.0f };
	
	ClientAssetConverter cac{ nullptr, nullptr, nullptr };
	cac.ConvertComponents(rifle, rifleModel);
	cac.ConvertComponents(avatarCtrl, avatarModel);

	Network * network = avatarCtrl->GetComponent<Network>();
	network->owner = playerId;
	network->replDesc = ClientCreateRemoteAvatarReplDesc(avatarCtrl->GetComponent<Script>()->GetScript<RemotePlayerScript>(0));
	network->id = objId;

	avatarCtrl->AddChild(rifleOrigin);
	rifleOrigin->AddChild(rifle);

	return avatarCtrl;
}

void GameClient::FetchUpdate() {
	Connection * connection = playerConnection.get();
	nn::Node<nn::GameMessage> * head = connection->sharedQueue.ConsumeAll();

	if(head == nullptr) {
		return;
	}

	Netcode::UndefinedBehaviourAssertion(connection->gameObject != nullptr &&
		connection->gameObject->HasComponent<Network>() &&
		connection->gameObject->HasComponent<Transform>());

	for(NodeIter<nn::GameMessage> it = head; it != nullptr; it++) {
		np::ServerUpdate * serverUpdate = it->allocator->MakeProto<np::ServerUpdate>();

		if(it->sequence <= connection->remoteGameSequence) {
			//Log::Debug("Dropping packed because its sequence is too low");
			continue;
		}
		
		if(!serverUpdate->ParseFromArray(it->content.Data(), static_cast<int32_t>(it->content.Size()))) {
			Log::Debug("Failed to parse from array");
			continue;
		}

		for(const np::ActionResult & ar : serverUpdate->action_results()) {
			ServerReconciliation sr;
			if(ConvertServerReconciliation(sr, ar)) {
				if(sr.id > connection->remoteActionIndex) {
					reconciliations.push_back(sr);
				}
			}
		}

		for(const np::ServerCommand & cmd : serverUpdate->commands()) {
			ServerReconciliation sr;
			if(ConvertServerReconciliation(sr, cmd)) {
				if(sr.id > connection->remoteCommandIndex) {
					reconciliations.push_back(sr);
				}
			}
		}

		google::protobuf::RepeatedPtrField<np::ReplData>* ptrField = serverUpdate->mutable_replications();

		for(np::ReplData& replData : *ptrField) {
			ReplData rd;
			rd.objectId = replData.object_id();
			rd.content = std::move(*replData.mutable_data());
			replicationData.emplace_back(std::move(rd));
		}

		connection->dtlsRoute->lastReceivedAt = Netcode::SystemClock::LocalNow();
		
		connection->redundancyBuffer.Confirm(serverUpdate->received_id());
		connection->remoteGameSequence = std::max(connection->remoteGameSequence, it->sequence);
	}
}

void GameClient::ProcessUpdate() {
	Connection * connection = playerConnection.get();
	
	std::sort(std::begin(reconciliations), std::end(reconciliations), [](const ServerReconciliation & a, const ServerReconciliation & b)->bool {
		return a.id < b.id;
	});
	
	for(const ServerReconciliation& sr : reconciliations) {
		if(sr.type == ReconciliationType::COMMAND) {
			if(sr.id > connection->remoteCommandIndex) {
				ProcessCommand(sr);
				connection->remoteCommandIndex = sr.id;
			}
		} else {
			if(sr.id > connection->remoteActionIndex) {
				ProcessResult(sr);
				connection->remoteActionIndex = sr.id;
			}
		}
	}

	reconciliations.clear();

	for(ReplData replData : replicationData) {
		for(GameObject * remoteObj : remoteObjects) {
			Network * network = remoteObj->GetComponent<Network>();

			if(network->id == replData.objectId) {
				if(network->owner != playerConnection->id) {
					ReplicateRead(remoteObj, network, replData.content, clock, connection->id, ActorType::CLIENT);
				}
				break;
			}
		}
	}

	replicationData.clear();
}

void GameClient::ProcessCommand(const ServerReconciliation & sr) {

	if(sr.type == ReconciliationType::COMMAND) {
		if(sr.command.type == CommandType::PLAYER_TIMEDOUT) {
			Log::Debug("Timedout: {0}", (int)sr.command.subject);
			RemoveRemoteObjectsByOwner(sr.command.subject);
		}

		if(sr.command.type == CommandType::CREATE_OBJECT) {
			GameScene * scene = Service::Get<GameSceneManager>()->GetScene();
			
			if(sr.command.subject == playerConnection->id) {
				GameObject* obj = playerConnection->gameObject;
				GameObject * camera = obj->Children().at(0);
				Log::Debug("Create local avatar: {0}", (int)sr.command.objectId);

				Network * network = obj->GetComponent<Network>();
				network->id = sr.command.objectId;
				network->replDesc = CreateLocalAvatarReplDesc(camera->GetComponent<Camera>());
				network->state = PlayerState::DEAD;

				Transform * transform = obj->GetComponent<Transform>();
				
				ReplicateRead(obj, network, sr.replData, clock, playerConnection->id, ActorType::CLIENT);

				physx::PxController* ctrl = localPlayerScript->GetController();
				ctrl->setFootPosition(ToPxExtVec3(transform->position));
			} else {
				
				if(sr.command.objectType == REPL_TYPE_REMOTE_AVATAR) {
					Log::Debug("Create remote avatar: {0}", (int)sr.command.objectId);
					GameObject * obj = ClientCreateRemoteAvatar(sr.command.subject, sr.command.objectId);
					ReplicateRead(obj, obj->GetComponent<Network>(), sr.replData, clock, playerConnection->id, ActorType::CLIENT);
					scene->SpawnWithHierarchy(obj);
					remoteObjects.emplace_back(obj);
				}

				if(sr.command.objectType == REPL_TYPE_SCOREBOARD) {
					if(scoreboard == nullptr) {
						Log::Debug("Create scoreboard: {0}", (int)sr.command.objectId);
						GameObject* sco = CreateScoreboard(sr.command.objectId);

						scoreboard = sco->GetComponent<Script>()
										->GetScript<ScoreboardScript>(0);

						remoteObjects.push_back(sco);
						
						scene->Spawn(sco);
					}
				}
			}
		}
	}
	
	/**
	 * TODO: Insert/Remove gameobjects or even close connection
	 */
}

void GameClient::ProcessResult(const ServerReconciliation & sr) {
	GameObject * gameObject = playerConnection->gameObject;
	Network * netw = gameObject->GetComponent<Network>();
	Transform * transform = gameObject->GetComponent<Transform>();

	// handle spawn accept
	if(sr.actionType == ActionType::SPAWN) {
		if(sr.type == ReconciliationType::ACCEPTED) {
			netw->state = PlayerState::ALIVE;
			transform->position = sr.movementCorrection.position;
			
			gameScene->SpawnWithHierarchy(gameObject);
			physx::PxController * ctrl = localPlayerScript->GetController();
			gameScene->GetPhysXScene()->addActor(*ctrl->getActor());
			ctrl->setFootPosition(ToPxExtVec3(transform->position));
		}
	}

	// handle movement rejection
	if(sr.actionType == ActionType::MOVEMENT) {
		if(sr.type == ReconciliationType::REJECTED) {
			Log::Debug("Correction");
			transform->position = sr.movementCorrection.position;
			localPlayerScript->GetController()->setFootPosition(ToPxExtVec3(transform->position));
		}
	}
}

void GameClient::RemoveRemoteObjectsByOwner(int32_t ownerId) {
	GameScene * scene = Service::Get<GameSceneManager>()->GetScene();

	auto it = std::remove_if(std::begin(remoteObjects), std::end(remoteObjects),
		[scene, ownerId](GameObject * gameObject) -> bool {
			Network * networkComponent = gameObject->GetComponent<Network>();
			if(networkComponent->owner == ownerId) {
				scene->RemoveWithHierarchy(gameObject);
				return true;
			}
			return false;
		});

	remoteObjects.erase(it, std::end(remoteObjects));
}

bool GameClient::IsConnected() const {
	return playerConnection != nullptr &&
		(playerConnection->state == nn::ConnectionState::ESTABLISHED ||
		playerConnection->state == nn::ConnectionState::SYNCHRONIZING);
}

bool GameClient::IncludeNetworkTick() {
	if(playerConnection == nullptr) {
		return false;
	}

	const uint32_t tc = playerConnection->tickCounter.load(std::memory_order_acquire);
	
	if(processedTick >= tc) {
		return false;
	}

	processedTick = tc;
	return true;
}

void GameClient::Receive() {

	FetchUpdate();

	ProcessUpdate();
	
}

static void AddAction(np::ClientUpdate* update, const RedundancyItem& item) {
	const ClientAction & action = std::get<ClientAction>(item.storage);
	
	np::ActionPrediction* prediction = update->add_predictions();
	prediction->set_type(static_cast<np::ActionType>(action.type));
	prediction->set_timestamp(Netcode::ConvertTimestampToUInt64(action.timestamp));
	prediction->set_id(action.id);

	if(action.type == ActionType::MOVEMENT) {
		np::Float3* pos = prediction->mutable_action_position();
		ConvertFloat3(pos, action.movementActionData.position);
	}

	if(action.type == ActionType::FIRE) {
		np::Float3 * pos = prediction->mutable_action_position();
		np::Float3 * dir = prediction->mutable_action_delta();
		ConvertFloat3(pos, action.fireActionData.position);
		ConvertFloat3(dir, action.fireActionData.direction);
	}
}

void GameClient::Send() {
	Ref<nn::NetAllocator> alloc = service->MakeAllocator(4096);
	
	if(playerConnection->state == nn::ConnectionState::SYNCHRONIZING) {
		np::Control * control = alloc->MakeProto<np::Control>();
		control->set_sequence(playerConnection->localControlSequence++);
		control->set_type(np::MessageType::CLOCK_SYNC_REQUEST);
		np::TimeSync* ts = control->mutable_time_sync();
		ts->set_client_req_transmission(Netcode::ConvertTimestampToUInt64(clock->GetLocalTime()));

		nn::ControlMessage cm;
		cm.allocator = std::move(alloc);
		cm.control = control;
		cm.packet = nullptr;
		
		service->Send(cm, playerConnection->dtlsRoute);
	} else {
		np::ClientUpdate * update = alloc->MakeProto<np::ClientUpdate>();

		update->set_received_id(playerConnection->remoteGameSequence);

		for(const RedundancyItem & item : playerConnection->redundancyBuffer.GetBuffer()) {
			AddAction(update, item);
		}

		GameObject * gameObj = playerConnection->gameObject;
		Network * network = gameObj->GetComponent<Network>();
		
		if(!network->replDesc.empty()) {
			std::string binary = ReplicateWrite(gameObj, network);

			if(!binary.empty()) {
				np::ReplData * rd = update->add_replications();
				rd->set_object_id(network->id);
				rd->set_data(std::move(binary));
			}
		}

		uint8_t * data = alloc->MakeArray<uint8_t>(update->ByteSizeLong());
		update->SerializeToArray(data, static_cast<int32_t>(update->ByteSizeLong()));

		nn::GameMessage gm;
		gm.sequence = playerConnection->localGameSequence++;
		gm.content = Netcode::ArrayView<uint8_t>{ data, update->ByteSizeLong() };
		gm.allocator = alloc;

		service->Send(gm, playerConnection.get());

		playerConnection->serverUpdate = nullptr;
		playerConnection->message = nn::GameMessage{};
	}
}

void GameClient::SendAction(ClientAction ca) {
	Connection * conn = playerConnection.get();
	ca.id = conn->localActionIndex++;
	conn->redundancyBuffer.Add(conn->localGameSequence, ca);
}

void GameClient::SendDebug() {
	static int x = 0;

	if(playerConnection->state == nn::ConnectionState::ESTABLISHED) {
		if(x == 0) {
			x++;
			SendAction(ClientAction::Spawn());
		} else {
			Script * script = playerConnection->gameObject->GetComponent<Script>();
			PlayerScript * playerScript = script->GetScript<PlayerScript>(0);

			SendAction(ClientAction::Move(ClientMovementActionData{
				playerConnection->gameObject->GetComponent<Transform>()->position
			}));
		}
	}

	Send();
}

nn::CompletionToken<nn::ClockSyncResult> GameClient::Synchronize() {
	nn::CompletionToken<nn::ClockSyncResult> ct = std::make_shared<nn::CompletionTokenType<nn::ClockSyncResult>>(&clientSession->GetIOContext());

	service->AddFilter(std::make_unique<ClockSyncResponseFilter>(clock, ct));
	
	return ct;
}

nn::CompletionToken<nn::TrResult> GameClient::ConnectionDone() {
	Ref<nn::NetAllocator> alloc = service->MakeAllocator(2048);
	np::Control * control = alloc->MakeProto<np::Control>();
	control->set_type(np::MessageType::CONNECT_DONE);
	control->set_sequence(playerConnection->localControlSequence++);
	control->mutable_connect_done()->set_measured_rtt(playerConnection->rtt.count());

	nn::ControlMessage cm;
	cm.allocator = std::move(alloc);
	cm.control = control;
	return service->Send(cm, playerConnection->dtlsRoute);
}

void GameClient::Start(Netcode::Module::INetworkModule * network, Netcode::GameClock* gameClock) {
	clientSession = std::dynamic_pointer_cast<nn::ClientSession>(network->CreateClient());
	playerConnection = std::make_shared<Connection>(clientSession->GetIOContext());
	scoreboard = nullptr;

	gameScene = Service::Get<GameSceneManager>()->GetScene();
	GameObject* obj = gameScene->FindByName("localAvatarRoot");

	localPlayerScript = obj->GetComponent<Script>()->GetScript<LocalPlayerScript>(0);

	clock = gameClock;
	playerConnection->gameObject = obj;
	
	const uint32_t intervalMs = Netcode::Config::GetOptional<uint32_t>(L"network.client.tickIntervalMs:u32", 250u);
	playerConnection->tickInterval.store(std::chrono::milliseconds{ intervalMs }, std::memory_order_release);
	
	clientSession->Connect(playerConnection, "localhost", 8889)->Then([this](const Netcode::ErrorCode & ec) -> void {
		if(ec) {
			Log::Error("Failed to connect: {0}", ec.message());
			return;
		}
		
		playerConnection->state = nn::ConnectionState::SYNCHRONIZING;
		service = clientSession->GetService();
		Synchronize()->Then([this](const nn::ClockSyncResult & csr) -> void {
			if(csr.errorCode) {
				Log::Error("Failed to synchronize: {0}", csr.errorCode.message());
				return;
			}
			
			playerConnection->rtt = nn::NtpClockFilter::DoubleToDuration(csr.delay);
			playerConnection->clockOffset = nn::NtpClockFilter::DoubleToDuration(csr.offset);
			clock->SynchronizeClocks(playerConnection->rtt, playerConnection->clockOffset);

			ConnectionDone()->Then([this](const nn::TrResult & tr) -> void {
				if(tr.errorCode) {
					Log::Error("Failed to send connection done: {0}", tr.errorCode.message());
					return;
				}
				
				playerConnection->state = nn::ConnectionState::ESTABLISHED;

				Log::Debug("Connection established");
			});
		});
	});
}
