#include "NetwUtil.h"
#include "../Services.h"
#include "ReplArguments.hpp"
#include "../Asset/ClientConverters.h"

np::ClientUpdate * ParseClientUpdate(nn::GameMessage * message) {
	if(message == nullptr) {
		return nullptr;
	}

	np::ClientUpdate * upd = message->allocator->MakeProto<np::ClientUpdate>();

	if(upd->ParseFromArray(message->content.Data(), static_cast<int32_t>(message->content.Size()))) {
		return upd;
	}

	return nullptr;
}

Netcode::Float3 ConvertFloat3(const np::Float3 & f3) {
	return Netcode::Float3{ f3.x(), f3.y(), f3.z() };
}

void ConvertFloat3(np::Float3 * dst, const Netcode::Float3 & src) {
	dst->set_x(src.x);
	dst->set_y(src.y);
	dst->set_z(src.z);
}

GameObject * CreateScoreboard(uint32_t id) {
	GameScene * scene = Service::Get<GameSceneManager>()->GetScene();
	GameObject * object = scene->Create("Scoreboard");
	Network * networkComponent = object->AddComponent<Network>();
	Script * script = object->AddComponent<Script>();
	std::unique_ptr<ScoreboardScript> ss = std::make_unique<ScoreboardScript>();
	networkComponent->id = id;
	networkComponent->owner = 0;
	networkComponent->replDesc = CreateScoreboardReplDesc(ss.get());
	script->AddScript(std::move(ss));
	return object;
}

GameObject * CreateRemoteAvatar(Netcode::Duration interpDelay, physx::PxControllerManager * controllerManager) {
	GameScene * gameScene = Service::Get<GameSceneManager>()->GetScene();
	
	GameObject * avatarController = gameScene->Create("remoteAvatarCtrl");
	
	auto [ctrlTransform, ctrlScript, ctrlNetwork, ctrlCamera] = avatarController->AddComponents<Transform, Script, Network, Camera>();
	
	ctrlCamera->ahead = Netcode::Float3::UnitZ;
	ctrlCamera->up = Netcode::Float3::UnitY;
	
	ctrlNetwork->state = PlayerState::SPECTATOR;
	ctrlNetwork->owner = -1;
	ctrlNetwork->id = 0;
	
	Netcode::PxPtr<physx::PxController> pxController = GameScene::CreateController(controllerManager);
	
	std::unique_ptr<RemotePlayerScript> rps = std::make_unique<RemotePlayerScript>(std::move(pxController), interpDelay);
	rps->Construct(avatarController);
	ctrlScript->AddScript(std::move(rps));

	return avatarController;
}

std::string ReplicateWrite(GameObject * gameObject, Network * networkComponent) {
	uint32_t requiredSize = 0;
	for(std::unique_ptr<ReplArgumentBase> & arg : networkComponent->replDesc) {
		requiredSize += arg->GetReplicatedSize(gameObject);
	}

	if(requiredSize == 0)
		return std::string{};

	std::string binary;
	binary.resize(requiredSize);

	Netcode::MutableArrayView<uint8_t> view{
		reinterpret_cast<uint8_t *>(binary.data()),
		binary.size()
	};
	
	for(std::unique_ptr<ReplArgumentBase> & arg : networkComponent->replDesc) {
		const uint32_t writtenSize = arg->Write(gameObject, view);

		if(writtenSize == 0)
			throw Netcode::UndefinedBehaviourException{ "Failed to write replication data" };

		view = view.Offset(writtenSize);
	}

	return binary;
}

void ReplicateRead(GameObject* gameObject, Network * networkComponent, const std::string & content, Netcode::GameClock * clock, int32_t connectionId, ActorType actor) {
	if(content.empty())
		return;
	
	Netcode::ArrayView<uint8_t> src{ reinterpret_cast<const uint8_t *>(content.data()), content.size() };

	for(std::unique_ptr<ReplArgumentBase> & arg : networkComponent->replDesc) {
		const uint32_t replicatedSize = arg->QueryReplicatedSize(src);
		const ReplType type = arg->GetType();

		if(replicatedSize == 0)
			throw Netcode::UndefinedBehaviourException{ "Failed to read from replication data" };

		/**
		 * Client predicted values are rejected by the server regardless of the owner
		 */
		if(actor == ActorType::SERVER && type == ReplType::CLIENT_PREDICTED) {
			src = src.Offset(replicatedSize);
			continue;
		}

		/**
		 * The owner client rejects the replication because it is the one predicting it
		 */
		if(actor == ActorType::CLIENT && type == ReplType::CLIENT_PREDICTED && connectionId == networkComponent->owner) {
			src = src.Offset(replicatedSize);
			continue;
		}
		
		const uint32_t readSize = arg->Read(gameObject, src);

		networkComponent->updatedAt = clock->GetLocalTime();

		if(replicatedSize != readSize)
			throw Netcode::UndefinedBehaviourException{ "Replication failed due to mismatching sizes" };
		
		src = src.Offset(readSize);
	}
}

ReplDesc CreateLocalAvatarReplDesc(Camera* cameraComponent) {
	ReplDesc replDesc;
	replDesc.emplace_back(ReplicateAsIsFromComponent(ReplType::CLIENT_PREDICTED, &Transform::position));
	replDesc.emplace_back(ReplicateAsIsFromState(ReplType::DEFAULT, cameraComponent, &Camera::ahead));
	return replDesc;
}

ReplDesc CreateScoreboardReplDesc(ScoreboardScript * scoreboard) {
	ReplDesc replDesc;
	replDesc.emplace_back(ReplicateAsIsFromState(ReplType::DEFAULT, scoreboard, &ScoreboardScript::stats));
	return replDesc;
}

ReplDesc ClientCreateRemoteAvatarReplDesc(RemotePlayerScript * rps) {
	ReplDesc replDesc;
	replDesc.emplace_back(ReplicateAsIsFromState(ReplType::CLIENT_PREDICTED, rps, &RemotePlayerScript::IND_position));
	replDesc.emplace_back(ReplicateAsIsFromState(ReplType::DEFAULT, rps, &RemotePlayerScript::IND_ahead));
	return replDesc;
}

ReplDesc CreateRemoteAvatarReplDesc() {
	ReplDesc replDesc;
	replDesc.emplace_back(ReplicateAsIsFromComponent(ReplType::CLIENT_PREDICTED, &Transform::position));
	replDesc.emplace_back(ReplicateAsIsFromComponent(ReplType::DEFAULT, &Camera::ahead));
	return replDesc;
}
