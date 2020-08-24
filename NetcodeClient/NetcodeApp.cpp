#include "NetcodeApp.h"
#include <Netcode/Input/AxisMap.hpp>
#include "DevCameraScript.h"
#include "PlayerBehavior.h"
#include "GunScript.h"
#include "DebugScript.h"
#include "RemoteAvatarScript.h"
#include "Snippets.h"
#include <NetcodeAssetLib/JsonUtility.h>

void GameApp::ReloadMap() {
	gameScene->Clear();
	graphics->ClearImportCache();

	LoadMap(mapAsset);
}

void GameApp::LoadGameObjectFromJson(const json11::Json::object & values) {
	AssetManager * assetManager = Service::Get<AssetManager>();
	GameObject * gameObj = nullptr;

	if(auto it = values.find("asset"); it != values.end()) {
		if(it->second.is_string()) {
			std::wstring assetPath = Netcode::Utility::ToWideString(it->second.string_value());

			gameObj = gameScene->Create();
			LoadComponents(assetManager->Import(std::move(assetPath)), gameObj);
		}
	}

	if(gameObj == nullptr) {
		return;
	}

	if(Transform * tr = gameObj->GetComponent<Transform>(); tr != nullptr) {
		if(auto it = values.find("position"); it != values.end() && it->second.is_array()) {
			tr->position = Netcode::Asset::LoadFloat3(it->second);
		}

		if(auto it = values.find("eulerAngles"); it != values.end() && it->second.is_array()) {
			tr->rotation = Netcode::Quaternion::FromEulerAngles(Netcode::Asset::LoadFloat3(it->second));
		}
	}

	gameScene->Spawn(gameObj);
}

void GameApp::LoadMap(const Netcode::URI::Model & path)
{
	mapAsset = path;

	json11::Json json = LoadJsonFile(mapAsset.GetModelPath());

	if(!json.is_object()) {
		return;
	}

	LoadAssets();

	const auto & rootObject = json.object_items();

	if(auto it = rootObject.find("objects"); it != rootObject.end()) {
		if(it->second.is_array()) {
			const auto & objArray = it->second.array_items();

			for(const auto & obj : objArray) {
				if(obj.is_object()) {
					LoadGameObjectFromJson(obj.object_items());
				}
			}
		}
	}
}

void GameApp::Render() {
	graphics->frame->Prepare();

	auto cfgBuilder = graphics->CreateFrameGraphBuilder();
	renderSystem.renderer.CreateComputeFrameGraph(cfgBuilder.get());
	graphics->frame->Run(cfgBuilder->Build(), FrameGraphCullMode::NONE);

	graphics->frame->DeviceSync();

	renderSystem.renderer.ReadbackComputeResults();
	renderSystem.renderer.perFrameData = &gameScene->perFrameData;
	renderSystem.renderer.ssaoData = &gameScene->ssaoData;

	gameScene->Foreach([this](GameObject * gameObject) -> void {
		if(gameObject->IsActive()) {
			transformSystem.Run(gameObject);
			pxSystem.Run(gameObject);
			renderSystem.Run(gameObject);
		}
	});

	gameScene->UpdatePerFrameCb();

	auto builder = graphics->CreateFrameGraphBuilder();
	renderSystem.renderer.CreateFrameGraph(builder.get());

	graphics->frame->Run(builder->Build(), FrameGraphCullMode::ANY);
	graphics->frame->Present();
	graphics->frame->DeviceSync();
	graphics->frame->CompleteFrame();

	renderSystem.renderer.Reset();
}

void GameApp::Simulate(float dt) {
	totalTime += dt;

	pageManager.Update(dt);

	gameScene->GetPhysXScene()->simulate(dt);
	gameScene->GetPhysXScene()->fetchResults(true);

	gameScene->Foreach([this, dt](GameObject * gameObject)->void {
		if(gameObject->IsActive()) {
			scriptSystem.Run(gameObject, dt);
			animSystem.Run(gameObject, dt);
		}
	});
}

void GameApp::LoadServices() {
	Service::Init<AssetManager>(graphics.get());
	Service::Init<GameScene>(px);

	gameScene = Service::Get<GameScene>();

	gameScene->Setup();
}

void GameApp::CreateUI() {
	Ref<LoginPage> loginPage = pageManager.CreatePage<LoginPage>(*px.physics);
	Ref<ServerBrowserPage> serverBrowserPage = pageManager.CreatePage<ServerBrowserPage>(*px.physics);
	Ref<LoadingPage> loadingPage = pageManager.CreatePage<LoadingPage>(*px.physics);

	loginPage->InitializeComponents();
	serverBrowserPage->InitializeComponents();
	loadingPage->InitializeComponents();

	pageManager.AddPage(loginPage);
	pageManager.AddPage(serverBrowserPage);
	pageManager.AddPage(loadingPage);

	renderSystem.renderer.ui_Input = &pageManager;
}

void GameApp::CreateAxisMapping() {
	Ref<Netcode::AxisMapBase> axisMap = std::make_shared<Netcode::AxisMap<AxisEnum>>(std::initializer_list<Netcode::AxisData<AxisEnum>> {
		Netcode::AxisData<AxisEnum> { AxisEnum::VERTICAL, Netcode::KeyCode::W, Netcode::KeyCode::S },
		Netcode::AxisData<AxisEnum> { AxisEnum::HORIZONTAL, Netcode::KeyCode::A, Netcode::KeyCode::D },
		Netcode::AxisData<AxisEnum> { AxisEnum::FIRE1, Netcode::KeyCode::MOUSE_LEFT, Netcode::KeyCode::UNDEFINED },
		Netcode::AxisData<AxisEnum> { AxisEnum::FIRE2, Netcode::KeyCode::MOUSE_RIGHT, Netcode::KeyCode::UNDEFINED },
		Netcode::AxisData<AxisEnum> { AxisEnum::JUMP, Netcode::KeyCode::SPACE, Netcode::KeyCode::UNDEFINED },
		Netcode::AxisData<AxisEnum> { AxisEnum::DEV_CAM_X, Netcode::KeyCode::NUM_6, Netcode::KeyCode::NUM_4 },
		Netcode::AxisData<AxisEnum> { AxisEnum::DEV_CAM_Y, Netcode::KeyCode::NUM_9, Netcode::KeyCode::NUM_7 },
		Netcode::AxisData<AxisEnum> { AxisEnum::DEV_CAM_Z, Netcode::KeyCode::NUM_8, Netcode::KeyCode::NUM_5 },
	});

	Netcode::Input::SetAxisMap(std::move(axisMap));

	Netcode::Input::OnKeyPressed->Subscribe([this](Netcode::Key key, Netcode::KeyModifier modifiers) -> void {
		if(modifiers == Netcode::KeyModifier::NONE) {
			if(key == Netcode::KeyCode::F5) {
				ReloadMap();
				Log::Debug("Map reloaded");
			}
		}
	});
}


void GameApp::LoadSystems() {
	renderSystem.CreatePermanentResources(graphics.get());
	defaultPhysxMaterial = px.physics->createMaterial(0.5f, 0.5f, 0.5f);
	animSystem.SetMovementController(&movCtrl);
	animSystem.renderer = &renderSystem.renderer;
}

void GameApp::LoadAssets() {
	AssetManager * assetManager = Service::Get<AssetManager>();

	CreateLocalAvatar();
	//CreateRemoteAvatar();

	{
		auto planeActor = physx::PxCreatePlane(*px.physics, physx::PxPlane{ 0.0f, 1.0f, 0.0f, 0.0f }, *defaultPhysxMaterial);
		gameScene->SpawnPhysxActor(planeActor);
	}
}

void GameApp::CreateRemoteAvatar() {
	AssetManager * assetManager = Service::Get<AssetManager>();

	GameObject * avatarController = gameScene->Create("remoteAvatarController");
	GameObject * avatarHitboxes = gameScene->Create("remoteAvatarHitboxes");

	Netcode::Asset::Model * avatarModel = assetManager->Import(L"compiled/models/ybot.ncasset");

	if(ybotAnimationSet == nullptr) {
		ybotAnimationSet = std::make_shared<AnimationSet>(graphics.get(), avatarModel->animations, avatarModel->bones);
	}

	/*
	GameObject * gunRootObj = gameScene->Create();
	GameObject * gunObj = gameScene->Create();
	LoadComponents(assetManager->Import(L"gun.ncasset"), gunObj);

	Transform * gunRootTransform = gunRootObj->AddComponent<Transform>();
	Transform * gunTransform = gunObj->GetComponent<Transform>();

	gunRootTransform->position = Netcode::Float3{ 0.0f, 130.0f, 0.0f };
	gunTransform->scale = Netcode::Float3{ 18.0f, 18.0f, 18.0f };
	gunRootObj->Parent(avatarController);
	gunObj->Parent(gunRootObj);

	Netcode::Quaternion gunRotation{ -Netcode::C_PIDIV2, -Netcode::C_PIDIV2, 0.0f };

	Script * gunScript = gunObj->AddComponent<Script>();

	auto behav = std::make_unique<GunBehavior>(avatarHitboxes, gunRootObj, Netcode::Float4{ 0.0f, 0.0f, 40.0f, 0.0f }, gunRotation, 28);

	GameObject * debugObj = gameScene->Create();
	Script * debugScript = debugObj->AddComponent<Script>();
	debugScript->SetBehavior(std::make_unique<DebugBehavior>(
	//&behav->localPosition.x, &behav->localPosition.y, &behav->localPosition.z
	&anim->effectors[0].position.x, &anim->effectors[0].position.y, &anim->effectors[0].position.z
	));
	debugScript->Setup(debugObj);


	gunScript->SetBehavior(
	std::move(behav)
	);
	gunScript->Setup(gunObj);

	gameScene->Spawn(gunRootObj);
	gameScene->Spawn(gunObj);
	gameScene->Spawn(debugObj);
	*/
	LoadComponents(avatarModel, avatarHitboxes);
	Animation * anim = avatarHitboxes->AddComponent<Animation>();
	CreateYbotAnimationComponent(avatarModel, anim);
	anim->blackboard->BindController(&movCtrl);
	anim->controller = ybotAnimationSet->CreateController();

	Netcode::PxPtr<physx::PxController> pxController = gameScene->CreateController();
	avatarController->AddComponent<Transform>();
	avatarController->AddComponent<Script>()->SetBehavior(std::make_unique<RemoteAvatarScript>(std::move(pxController)));
	avatarHitboxes->Parent(avatarController);

	gameScene->Spawn(avatarController);
	gameScene->Spawn(avatarHitboxes);
}



void GameApp::CreateLocalAvatar() {
	GameObject * avatarController = gameScene->Create("localAvatarController");
	GameObject * avatarCamera = gameScene->Create("localAvatarCamera");

	avatarCamera->Parent(avatarController);

	Transform * avatarCamTransform = avatarCamera->AddComponent<Transform>();
	avatarCamTransform->position.y = 180.0f;

	Camera * fpsCam = avatarCamera->AddComponent<Camera>();
	fpsCam->ahead = Netcode::Float3{ 0.0f, 0.0f, 1.0f };
	fpsCam->aspect = graphics->GetAspectRatio();
	fpsCam->farPlane = 10000.0f;
	fpsCam->nearPlane = 1.0f;
	fpsCam->up = Netcode::Float3{ 0.0f, 1.0f, 0.0f };

	Netcode::PxPtr<physx::PxController> pxController = gameScene->CreateController();

	Transform * act = avatarController->AddComponent<Transform>();
	act->position = Netcode::Float3{ 0.0f, 0.0f, 200.0f };

	Script * scriptComponent = avatarController->AddComponent<Script>();
	scriptComponent->SetBehavior(std::make_unique<PlayerBehavior>(std::move(pxController), fpsCam));
	scriptComponent->Setup(avatarController);

	gameScene->Spawn(avatarController);
	gameScene->Spawn(avatarCamera);

	gameScene->SetCamera(avatarCamera);
}

void GameApp::LoadComponents(Netcode::Asset::Model * model, GameObject * gameObject) {
	gameObject->AddComponent<Transform>();

	if(model->meshes.Size() > 0) {
		Model * modelComponent = gameObject->AddComponent<Model>();
		LoadModelComponent(model, modelComponent);

		if(model->bones.Size() > 0 && model->animations.Size() > 0) {

		}
	}

	if(model->colliders.Size() > 0) {
		Collider * colliderComponent = gameObject->AddComponent<Collider>();
		LoadColliderComponent(model, colliderComponent);
		colliderComponent->actor->userData = gameObject;
	}
}

void GameApp::LoadColliderComponent(Netcode::Asset::Model * model, Collider * colliderComponent) {
	if(model->colliders.Size() == 0) {
		return;
	}

	physx::PxPhysics * pxp = px.physics.Get();
	Netcode::PxPtr<physx::PxRigidDynamic> actor = pxp->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
	actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
	std::vector<ColliderShape> netcodeShapes;
	netcodeShapes.reserve(model->colliders.Size());

	for(size_t i = 0; i < model->colliders.Size(); ++i) {
		const Netcode::Asset::Collider * netcodeCollider = model->colliders.Data() + i;
		netcodeShapes.push_back(*netcodeCollider);
		Netcode::PxPtr<physx::PxShape> shape{ nullptr };
		physx::PxShapeFlags shapeFlags;
		physx::PxFilterData filterData;

		if(netcodeCollider->boneReference >= 0 && netcodeCollider->boneReference < 0x7F) {
			shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eTRIGGER_SHAPE | physx::PxShapeFlag::eVISUALIZATION;
			filterData.word0 = PHYSX_COLLIDER_TYPE_HITBOX;
		} else {
			shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION;
			filterData.word0 = PHYSX_COLLIDER_TYPE_WORLD;
		}

		if(netcodeCollider->type == Netcode::Asset::ColliderType::MESH) {
			const auto & mesh = model->meshes[0];
			physx::PxConvexMeshDesc cmd;
			cmd.points.count = mesh.lodLevels[0].vertexCount;
			cmd.points.data = mesh.vertices;
			cmd.points.stride = mesh.vertexSize;
			cmd.vertexLimit = 255;
			cmd.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX | physx::PxConvexFlag::eFAST_INERTIA_COMPUTATION;
			cmd.indices.data = mesh.indices;
			cmd.indices.count = mesh.lodLevels[0].indexCount;
			cmd.indices.stride = 4;

			physx::PxDefaultMemoryOutputStream buf;
			physx::PxConvexMeshCookingResult::Enum r;
			px.cooking->cookConvexMesh(cmd, buf, &r);

			physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
			Netcode::PxPtr<physx::PxConvexMesh> convexMesh = pxp->createConvexMesh(input);

			physx::PxConvexMeshGeometry pcmg{ convexMesh.Get() };

			shape = pxp->createShape(pcmg, *defaultPhysxMaterial, true, shapeFlags);

		} else {
			shape = CreatePrimitiveShapeFromAsset(*netcodeCollider, pxp, defaultPhysxMaterial.Get(), shapeFlags);
		}

		shape->setLocalPose(physx::PxTransform{
				Netcode::ToPxVec3(netcodeCollider->localPosition),
				Netcode::ToPxQuat(netcodeCollider->localRotation)
			});

		shape->setQueryFilterData(filterData);
		shape->userData = nullptr;
		actor->attachShape(*shape);
	}

	colliderComponent->actor = actor.Release();
	colliderComponent->shapes = std::move(netcodeShapes);
}

void GameApp::LoadModelComponent(Netcode::Asset::Model * model, Model * modelComponent) {
	for(size_t meshIdx = 0; meshIdx < model->meshes.Size(); ++meshIdx) {
		const Netcode::Asset::Material * mat = model->materials.Data() + model->meshes[meshIdx].materialId;
		const Netcode::Asset::Mesh * mesh = model->meshes.Data() + meshIdx;

		modelComponent->perObjectData.Model = Netcode::Float4x4::Identity;
		modelComponent->perObjectData.InvModel = Netcode::Float4x4::Identity;

		auto batch = graphics->resources->CreateUploadBatch();

		auto appMesh = std::make_shared<Mesh>();

		uint8_t * const vBasePtr = reinterpret_cast<uint8_t *>(mesh->vertices);
		uint8_t * const iBasePtr = reinterpret_cast<uint8_t *>(mesh->indices);

		for(unsigned int i = 0; i < mesh->lodLevelsLength; ++i) {
			uint8_t * vData = vBasePtr + mesh->lodLevels[i].vertexBufferByteOffset;
			uint8_t * iData = nullptr;
			Ref<Netcode::GpuResource> vbuffer = graphics->resources->CreateVertexBuffer(mesh->lodLevels[i].vertexBufferSizeInBytes, mesh->vertexSize, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
			uint64_t vCount = mesh->lodLevels[i].vertexCount;
			Ref<Netcode::GpuResource> ibuffer{ nullptr };
			uint64_t iCount = 0;

			batch->Upload(vbuffer, vData, mesh->lodLevels[i].vertexBufferSizeInBytes);
			batch->Barrier(vbuffer, ResourceState::COPY_DEST, ResourceState::VERTEX_AND_CONSTANT_BUFFER);

			if(mesh->indices != nullptr) {
				ibuffer = graphics->resources->CreateIndexBuffer(mesh->lodLevels[i].indexBufferSizeInBytes, DXGI_FORMAT_R32_UINT, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
				iData = iBasePtr + mesh->lodLevels[i].indexBufferByteOffset;
				iCount = mesh->lodLevels[i].indexCount;

				batch->Upload(ibuffer, iData, mesh->lodLevels[i].indexBufferSizeInBytes);
				batch->Barrier(ibuffer, ResourceState::COPY_DEST, ResourceState::INDEX_BUFFER);
			}

			GBuffer lod;
			lod.indexBuffer = ibuffer;
			lod.vertexBuffer = vbuffer;
			lod.indexCount = iCount;
			lod.vertexCount = vCount;

			appMesh->AddLOD(lod);
			appMesh->vertexSize = mesh->vertexSize;
		}

		appMesh->selectedLod = 0;
		appMesh->boundingBox = mesh->boundingBox;

		graphics->frame->SyncUpload(std::move(batch));

		std::string ncMatName{ std::string_view{ mat->name, 48 } };
		Ref<Netcode::Material> ncMat = std::make_shared<Netcode::BrdfMaterial>(static_cast<Netcode::MaterialType>(mat->type), ncMatName);

		for(uint32_t j = 0; j < mat->indicesLength; ++j) {
			Netcode::Asset::MaterialParamIndex param = mat->indices[j];
			void * pptr = ncMat->GetParameterPointer(param.id);

			if(param.id < static_cast<uint32_t>(Netcode::MaterialParamId::SENTINEL_TEXTURE_PATHS_BEGIN)) {
				memcpy(pptr, mat->data + param.offset, param.size);
			} else if(param.id < static_cast<uint32_t>(Netcode::MaterialParamId::SENTINEL_TEXTURE_PATHS_END)) {
				if(param.size > 0) {
					std::wstring fullUriPath{ std::wstring_view{
						reinterpret_cast<wchar_t *>(mat->data + param.offset),
						param.size / sizeof(wchar_t)
					}
					};

					Netcode::URI::Texture texUri{ std::move(fullUriPath), Netcode::FullPathToken{ } };

					(*reinterpret_cast<Netcode::URI::Texture *>(pptr)) = std::move(texUri);
				}
			}
		}

		modelComponent->AddShadedMesh(appMesh, ncMat);
	}

}
