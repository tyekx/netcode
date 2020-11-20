#include "GameClient.h"
#include <Netcode/Network/Service.h>
#include "../Services.h"
#include "../Asset/ClientConverters.h"
#include "../AnimationSet.h"
#include <Netcode/MovementController.h>

static bool ConvertServerReconciliation(ServerReconciliation& dst, const np::ServerCommand& command) {
	ServerReconciliation sr = {};
	sr.type = ReconciliationType::COMMAND;
	sr.actionType = ActionType::NOOP;
	sr.id = command.id();
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
	static Netcode::MovementController movCtrl;

	GameScene * scene = Service::Get<GameSceneManager>()->GetScene();
	
	GameObject * gameObject = CreateRemoteAvatar();
	GameObject * avatarRifle = scene->Create("remoteRifle");

	Transform * rifleTransform = avatarRifle->AddComponent<Transform>();
	rifleTransform->rotation = Netcode::Quaternion{ 0.0f, -Netcode::C_PIDIV2, 0.0f };

	Network * network = gameObject->GetComponent<Network>();
	network->owner = playerId;
	network->replDesc = ClientCreateRemoteAvatarReplDesc(gameObject->GetComponent<Script>()->GetScript<RemotePlayerScript>(0));
	network->id = objId;
	
	ClientAssetConverter cac{ nullptr, nullptr, nullptr };
	AssetManager * assetManager = Service::Get<AssetManager>();
	Netcode::Asset::Model * ncModel = assetManager->Import(L"compiled/models/ybot.ncasset");
	Netcode::Asset::Model * rifleModel = assetManager->Import(L"compiled/models/gun_2.ncasset");
	cac.ConvertComponents(gameObject, ncModel);
	cac.ConvertComponents(avatarRifle, rifleModel);

	if(remoteAvatarAnimationSet == nullptr) {
		remoteAvatarAnimationSet = std::make_shared<AnimationSet>(Service::Get<Netcode::Module::IGraphicsModule *>(), ncModel->animations, ncModel->bones);
	}
	
	Animation * anim = gameObject->AddComponent<Animation>();
	CreateYbotAnimationComponent(ncModel, anim);
	anim->blackboard->BindController(&movCtrl);
	anim->controller = remoteAvatarAnimationSet->CreateController();

	Script * script = avatarRifle->AddComponent<Script>();
	std::unique_ptr<SocketScript> ss = std::make_unique<SocketScript>();
	ss->boneId = 28;
	ss->anim = anim;
	ss->offset = Netcode::Float3{ -34.0f, -3.0f, 4.0f };
	script->AddScript(std::move(ss));

	gameObject->AddChild(avatarRifle);

	return gameObject;
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

	for(NodeIter<nn::GameMessage> it = head; it != nullptr; ++it) {
		np::ServerUpdate * serverUpdate = it->allocator->MakeProto<np::ServerUpdate>();

		if(it->sequence <= connection->remoteGameSequence) {
			continue;
		}
		
		if(!serverUpdate->ParseFromArray(it->content.Data(), static_cast<int32_t>(it->content.Size()))) {
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
}

void GameClient::ProcessCommand(const ServerReconciliation & sr) {

	if(sr.type == ReconciliationType::COMMAND) {
		if(sr.command.type == CommandType::PLAYER_TIMEDOUT) {
			Log::Debug("Timedout: {0}", (int)sr.command.subject);
			RemoveRemoteObjectsByOwner(sr.command.subject);
		}

		if(sr.command.type == CommandType::CREATE_OBJECT) {
			if(sr.command.subject == playerConnection->id) {
				GameObject* obj = playerConnection->gameObject;
				GameObject * camera = obj->Children().at(0);

				Network * network = obj->GetComponent<Network>();
				network->id = sr.command.objectId;
				network->replDesc = CreateLocalAvatarReplDesc(camera->GetComponent<Camera>());
			} else {
				GameObject * obj = ClientCreateRemoteAvatar(sr.command.subject, sr.command.objectId);

				GameScene * scene = Service::Get<GameSceneManager>()->GetScene();
				scene->SpawnWithHierarchy(obj);
				remoteObjects.emplace_back(obj);
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
		}
	}

	// handle movement rejection
	if(sr.actionType == ActionType::MOVEMENT) {
		if(sr.type == ReconciliationType::REJECTED) {
			transform->position = sr.movementCorrection.position;
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
	return playerConnection != nullptr && playerConnection->state == nn::ConnectionState::ESTABLISHED;
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
	np::ActionPrediction* prediction = update->add_predictions();
	prediction->set_type(static_cast<np::ActionType>(item.action.type));
	prediction->set_timestamp(Netcode::ConvertTimestampToUInt64(item.action.timestamp));
	prediction->set_id(item.action.id);

	if(item.action.type == ActionType::MOVEMENT) {
		np::Float3* pos = prediction->mutable_action_position();
		ConvertFloat3(pos, item.action.movementActionData.position);
	}

	if(item.action.type == ActionType::FIRE) {
		np::Float3 * pos = prediction->mutable_action_position();
		np::Float3 * dir = prediction->mutable_action_delta();
		ConvertFloat3(pos, item.action.fireActionData.position);
		ConvertFloat3(dir, item.action.fireActionData.direction);
	}

	if(item.action.type == ActionType::LOOK) {
		np::Float3 * pos = prediction->mutable_action_position();
		ConvertFloat3(pos, Netcode::Float3{
			item.action.lookActionData.pitch,
			item.action.lookActionData.yaw, 0.0f });
	}
}

void GameClient::Send() {
	Ref<nn::NetcodeService> service = clientSession->GetService();
	Ref<nn::NetAllocator> alloc = service->MakeAllocator(4096);
	np::ClientUpdate * update = alloc->MakeProto<np::ClientUpdate>();

	update->set_received_id(playerConnection->remoteGameSequence);

	for(const RedundancyItem& item : playerConnection->redundancyBuffer.GetBuffer()) {
		AddAction(update, item);
	}

	GameObject * gameObj = playerConnection->gameObject;
	Network * network = gameObj->GetComponent<Network>();

	if(network->replDesc != nullptr) {
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
}

void GameClient::SendAction(ClientAction ca) {
	Connection * conn = playerConnection.get();
	ca.id = conn->localActionIndex++;
	conn->redundancyBuffer.Add(conn->localGameSequence, ca);
}

void GameClient::SendDebug() {
	static int x = 0;

	if(x == 0) {
		x++;
		SendAction(ClientAction::Spawn());
	} else {
		Script * script = playerConnection->gameObject->GetComponent<Script>();
		PlayerScript * playerScript = script->GetScript<PlayerScript>(0);
		
		SendAction(ClientAction::Look(ClientLookActionData{
			playerScript->cameraPitch,
			playerScript->cameraYaw
		}));
		
		SendAction(ClientAction::Move(ClientMovementActionData{
			playerConnection->gameObject->GetComponent<Transform>()->position
		}));
	}

	Send();
}

void GameClient::Start(Netcode::Module::INetworkModule * network, Netcode::GameClock* gameClock) {
	clientSession = std::dynamic_pointer_cast<nn::ClientSession>(network->CreateClient());
	playerConnection = std::make_shared<Connection>(clientSession->GetIOContext());

	GameSceneManager* gsm = Service::Get<GameSceneManager>();
	GameObject* obj = gsm->GetScene()->FindByName("localAvatarRoot");

	clock = gameClock;
	playerConnection->gameObject = obj;
	
	clientSession->Connect(playerConnection, "localhost", 8889)->Then([this](const Netcode::ErrorCode & ec) -> void {
		Log::Debug("Connection done: {0}", ec.message());
		//playerConnection->tickInterval.store(std::chrono::microseconds{ 16666 }, std::memory_order_release);
		playerConnection->state = nn::ConnectionState::ESTABLISHED;
	});
}
