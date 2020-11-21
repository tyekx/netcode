#include "NetwUtil.h"
#include "../Services.h"
#include "ReplArguments.hpp"

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


GameObject * CreateRemoteAvatar(Netcode::Duration interpDelay) {
	AssetManager * assetManager = Service::Get<AssetManager>();
	GameSceneManager * gsm = Service::Get<GameSceneManager>();
	GameScene * gameScene = gsm->GetScene();
	
	GameObject * avatarController = gameScene->Create();
	GameObject * avatarHitboxes = gameScene->Create("remoteAvatarHitboxes");

	avatarController->AddComponent<Transform>();
	Script * scriptComponent = avatarController->AddComponent<Script>();
	Network * networkComponent = avatarController->AddComponent<Network>();
	Camera * cameraComponent = avatarController->AddComponent<Camera>();
	cameraComponent->ahead = Netcode::Float3::UnitZ;
	cameraComponent->up = Netcode::Float3::UnitY;
	networkComponent->state = PlayerState::SPECTATOR;
	networkComponent->owner = -1;
	networkComponent->id = 0;
	networkComponent->replDesc = CreateRemoteAvatarReplDesc();
	
	Netcode::PxPtr<physx::PxController> pxController = gameScene->CreateController();
	std::unique_ptr<RemotePlayerScript> rps = std::make_unique<RemotePlayerScript>(std::move(pxController), interpDelay);
	rps->Construct(avatarController);
	scriptComponent->AddScript(std::move(rps));

	avatarController->AddChild(avatarHitboxes);
	//Netcode::Asset::Model * avatarModel = assetManager->Import(L"compiled/models/ybot.ncasset");

	return avatarController;
}

std::string ReplicateWrite(GameObject * gameObject, Network * networkComponent) {
	uint32_t requiredSize = 0;
	for(std::unique_ptr<ReplArgumentBase> & arg : networkComponent->replDesc->arguments) {
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
	
	for(std::unique_ptr<ReplArgumentBase> & arg : networkComponent->replDesc->arguments) {
		const uint32_t writtenSize = arg->Write(gameObject, view);

		if(writtenSize == 0)
			throw Netcode::UndefinedBehaviourException{ "Failed to write replication data" };

		view = view.Offset(writtenSize);
	}

	return binary;
}

void ReplicateRead(GameObject* gameObject, Network * networkComponent, const std::string & content, Netcode::GameClock * clock, int32_t connectionId, ActorType actor) {
	Netcode::ArrayView<uint8_t> src{ reinterpret_cast<const uint8_t *>(content.data()), content.size() };

	for(std::unique_ptr<ReplArgumentBase> & arg : networkComponent->replDesc->arguments) {
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

Ref<ReplDesc> CreateLocalAvatarReplDesc(Camera* cameraComponent) {
	Ref<ReplDesc> replDesc = std::make_shared<ReplDesc>();
	replDesc->arguments.emplace_back(ReplicateAsIsFromComponent(ReplType::CLIENT_PREDICTED, &Transform::position));
	replDesc->arguments.emplace_back(ReplicateAsIsFromState(ReplType::DEFAULT, cameraComponent, &Camera::ahead));
	return replDesc;
}

Ref<ReplDesc> ClientCreateRemoteAvatarReplDesc(RemotePlayerScript * rps) {
	Ref<ReplDesc> replDesc = std::make_shared<ReplDesc>();
	replDesc->arguments.emplace_back(ReplicateAsIsFromState(ReplType::CLIENT_PREDICTED, rps, &RemotePlayerScript::IND_position));
	replDesc->arguments.emplace_back(ReplicateAsIsFromState(ReplType::DEFAULT, rps, &RemotePlayerScript::IND_ahead));
	return replDesc;
}

Ref<ReplDesc> CreateRemoteAvatarReplDesc() {
	Ref<ReplDesc> replDesc = std::make_shared<ReplDesc>();
	replDesc->arguments.emplace_back(ReplicateAsIsFromComponent(ReplType::CLIENT_PREDICTED, &Transform::position));
	replDesc->arguments.emplace_back(ReplicateAsIsFromComponent(ReplType::DEFAULT, &Camera::ahead));
	return replDesc;
}
