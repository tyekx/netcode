#pragma once

#include <DirectXColors.h>

#include <Netcode/Input.h>
#include <Netcode/BasicGeometry.h>
#include <Netcode/DebugPhysx.h>
#include <Netcode/MathExt.h>
#include <Netcode/Modules.h>
#include <Netcode/Stopwatch.h>
#include <Netcode/Service.hpp>
#include <Netcode/UI/PageManager.h>

#include "Asset.h"
#include "GameObject.h"
#include "Systems.h"
#include "Scene.h"
#include "DevCameraScript.h"
#include "PlayerBehavior.h"
#include "GunScript.h"
#include "DebugScript.h"
#include "Snippets.h"
#include "PhysxHelpers.h"
#include "Services.h"
#include "RemoteAvatarScript.h"
#include "UITest.h"

using Netcode::Graphics::ResourceType;
using Netcode::Graphics::ResourceState;
using Netcode::Graphics::FrameGraphCullMode;

class GameApp : public Netcode::Module::AApp, Netcode::Module::TAppEventHandler {
	Netcode::Stopwatch stopwatch;
	Netcode::Physics::PhysX px;
	Netcode::PxPtr<physx::PxMaterial> defaultPhysxMaterial;
	
	Netcode::MovementController movCtrl;

	TransformSystem transformSystem;
	ScriptSystem scriptSystem;
	RenderSystem renderSystem;
	AnimationSystem animSystem;
	PhysXSystem pxSystem;
	Netcode::UI::PageManager pageManager;

	GameScene * gameScene;

	std::shared_ptr<AnimationSet> ybotAnimationSet;
	Netcode::Network::GameSessionRef gameSession;

	float totalTime;

	void LoadSystems() {

	}

	void Render() {

		graphics->frame->Prepare();

		auto cfgBuilder = graphics->CreateFrameGraphBuilder();
		renderSystem.renderer.CreateComputeFrameGraph(cfgBuilder);
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
		renderSystem.renderer.CreateFrameGraph(builder);

		graphics->frame->Run(builder->Build(), FrameGraphCullMode::ANY);
		graphics->frame->Present();
		graphics->frame->DeviceSync();
		graphics->frame->CompleteFrame();
		
		renderSystem.renderer.Reset();
	}

	void Simulate(float dt) {
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

	void LoadServices() {
		Service::Init<AssetManager>(graphics.get());
		Service::Init<GameScene>(px);

		gameScene = Service::Get<GameScene>();

		gameScene->Setup();
	}

	void ConnectServer() {
		gameSession = network->CreateClient();
	}

	void LoadAssets() {
		AssetManager * assetManager = Service::Get<AssetManager>();

		Netcode::AxisMapRef axisMap = std::make_shared<Netcode::AxisMap<AxisEnum>>(std::initializer_list<Netcode::AxisData<AxisEnum>> {
			Netcode::AxisData<AxisEnum> { AxisEnum::VERTICAL,		Netcode::KeyCode::W,			Netcode::KeyCode::S },
			Netcode::AxisData<AxisEnum> { AxisEnum::HORIZONTAL,	Netcode::KeyCode::D,			Netcode::KeyCode::A },
			Netcode::AxisData<AxisEnum> { AxisEnum::FIRE1,			Netcode::KeyCode::MOUSE_LEFT,	Netcode::KeyCode::UNDEFINED },
			Netcode::AxisData<AxisEnum> { AxisEnum::FIRE2,			Netcode::KeyCode::MOUSE_RIGHT,	Netcode::KeyCode::UNDEFINED },
			Netcode::AxisData<AxisEnum> { AxisEnum::JUMP,			Netcode::KeyCode::SPACE,		Netcode::KeyCode::UNDEFINED },
			Netcode::AxisData<AxisEnum> { AxisEnum::DEV_CAM_X,		Netcode::KeyCode::NUM_6,		Netcode::KeyCode::NUM_4 },
			Netcode::AxisData<AxisEnum> { AxisEnum::DEV_CAM_Y,		Netcode::KeyCode::NUM_9,		Netcode::KeyCode::NUM_7 },
			Netcode::AxisData<AxisEnum> { AxisEnum::DEV_CAM_Z,		Netcode::KeyCode::NUM_8,		Netcode::KeyCode::NUM_5 },
		});

		Netcode::Input::SetAxisMap(std::move(axisMap));

		renderSystem.CreatePermanentResources(graphics.get());
		defaultPhysxMaterial = px.physics->createMaterial(0.5f, 0.5f, 0.5f);
		animSystem.renderer = &renderSystem.renderer;

		CreateLocalAvatar();
		CreateRemoteAvatar();

		animSystem.SetMovementController(&movCtrl);

		{
			auto planeActor = physx::PxCreatePlane(*px.physics, physx::PxPlane{ 0.0f, 1.0f, 0.0f, 200.0f }, *defaultPhysxMaterial);
			gameScene->SpawnPhysxActor(planeActor);
		}


		std::shared_ptr<LoginPage> loginPage = pageManager.CreatePage<LoginPage>(px);
		loginPage->InitializeComponents();

		std::shared_ptr<ServerBrowserPage> serverBrowserPage = pageManager.CreatePage<ServerBrowserPage>(px);
		serverBrowserPage->InitializeComponents();

		pageManager.AddPage(loginPage);
		pageManager.AddPage(serverBrowserPage);
		pageManager.Activate(PagesEnum::LOGIN_PAGE);
		renderSystem.renderer.ui_Input = loginPage;

		GameObject * testbox = gameScene->Create();
		
		LoadComponents(assetManager->Import(L"testbox.ncasset"), testbox);


		gameScene->Spawn(testbox);
	}

	void CreateRemoteAvatar() {
		AssetManager * assetManager = Service::Get<AssetManager>();

		GameObject * avatarController = gameScene->Create();
		GameObject * avatarHitboxes = gameScene->Create();

		Netcode::Asset::Model * avatarModel = assetManager->Import(L"ybot.ncasset");

		if(ybotAnimationSet == nullptr) {
			ybotAnimationSet = std::make_shared<AnimationSet>(graphics.get(), avatarModel->animations, avatarModel->bones);
		}

		GameObject * gunRootObj = gameScene->Create();
		GameObject * gunObj = gameScene->Create();
		LoadComponents(assetManager->Import(L"gun.ncasset"), gunObj);

		Transform * gunRootTransform = gunRootObj->AddComponent<Transform>();
		Transform * gunTransform = gunObj->GetComponent<Transform>();

		gunRootTransform->position = Netcode::Float3{ 0.0f, 130.0f, 0.0f };
		gunTransform->scale = Netcode::Float3{ 18.0f, 18.0f, 18.0f };

		LoadComponents(avatarModel, avatarHitboxes);
		Animation* anim = avatarHitboxes->AddComponent<Animation>();
		CreateYbotAnimationComponent(avatarModel, anim);
		anim->blackboard->BindController(&movCtrl);
		anim->controller = ybotAnimationSet->CreateController();

		physx::PxController * pxController = gameScene->CreateController();
		avatarController->AddComponent<Transform>();
		avatarHitboxes->Parent(avatarController);
		gunRootObj->Parent(avatarController);
		gunObj->Parent(gunRootObj);

		avatarController->AddComponent<Script>()->SetBehavior(std::make_unique<RemoteAvatarScript>(pxController));

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

		gameScene->Spawn(avatarController);
		gameScene->Spawn(avatarHitboxes);
		gameScene->Spawn(gunRootObj);
		gameScene->Spawn(gunObj);
		gameScene->Spawn(debugObj);
	}

	void CreateLocalAvatar() {
		GameObject * avatarController = gameScene->Create();
		GameObject * avatarCamera = gameScene->Create();

		avatarCamera->Parent(avatarController);

		Transform* avatarCamTransform =  avatarCamera->AddComponent<Transform>();
		avatarCamTransform->position.y = 180.0f;

		Camera * fpsCam = avatarCamera->AddComponent<Camera>();
		fpsCam->ahead = Netcode::Float3{ 0.0f, 0.0f, 1.0f };
		fpsCam->aspect = graphics->GetAspectRatio();
		fpsCam->farPlane = 10000.0f;
		fpsCam->nearPlane = 1.0f;
		fpsCam->up = Netcode::Float3{ 0.0f, 1.0f, 0.0f };

		physx::PxController * pxController = gameScene->CreateController();

		Transform * act = avatarController->AddComponent<Transform>();
		act->position = Netcode::Float3{ 0.0f, 0.0f, 200.0f };

		Script* scriptComponent = avatarController->AddComponent<Script>();
		scriptComponent->SetBehavior(std::make_unique<PlayerBehavior>(pxController, fpsCam));
		scriptComponent->Setup(avatarController);

		gameScene->Spawn(avatarController);
		gameScene->Spawn(avatarCamera);

		gameScene->SetCamera(avatarCamera);
	}

	void LoadComponents(Netcode::Asset::Model * model, GameObject * gameObject) {
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
			colliderComponent->actorRef->userData = gameObject;
		}
	}

	void LoadColliderComponent(Netcode::Asset::Model * model, Collider * colliderComponent) {
		if(model->colliders.Size() == 0) {
			return;
		}

		physx::PxPhysics * pxp = px.physics.Get();
		physx::PxRigidDynamic * actor = pxp->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
		actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
		std::vector<ColliderShape> netcodeShapes;
		netcodeShapes.reserve(model->colliders.Size());

		for(size_t i = 0; i < model->colliders.Size(); ++i) {
			const Netcode::Asset::Collider * netcodeCollider = model->colliders.Data() + i;
			netcodeShapes.push_back(*netcodeCollider);
			physx::PxShape * shape = nullptr;
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
				physx::PxConvexMesh * convexMesh = pxp->createConvexMesh(input);

				physx::PxConvexMeshGeometry pcmg{ convexMesh };

				shape = pxp->createShape(pcmg, *defaultPhysxMaterial, true, shapeFlags);

			} else {
				shape = CreatePrimitiveShapeFromAsset(*netcodeCollider, pxp, defaultPhysxMaterial.Get(), shapeFlags);
			}

			shape->setQueryFilterData(filterData);
			shape->userData = &netcodeShapes.at(i);
			actor->attachShape(*shape);
		}

		colliderComponent->actorRef = actor;
		colliderComponent->shapes = std::move(netcodeShapes);
	}

	void LoadModelComponent(Netcode::Asset::Model * model, Model * modelComponent) {
		for(size_t meshIdx = 0; meshIdx < model->meshes.Size(); ++meshIdx) {
			const Netcode::Asset::Material * mat = model->materials.Data() + model->meshes[meshIdx].materialId;
			const Netcode::Asset::Mesh * mesh = model->meshes.Data() + meshIdx;

			DirectX::XMStoreFloat4x4(&modelComponent->perObjectData.Model, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4(&modelComponent->perObjectData.InvModel, DirectX::XMMatrixIdentity());

			Netcode::InputLayoutBuilderRef inputLayoutBuilder = graphics->CreateInputLayoutBuilder();

			for(uint32_t ilIdx = 0; ilIdx < mesh->inputElementsLength; ++ilIdx) {
				inputLayoutBuilder->AddInputElement(mesh->inputElements[ilIdx].semanticName,
					mesh->inputElements[ilIdx].semanticIndex,
					mesh->inputElements[ilIdx].format,
					mesh->inputElements[ilIdx].byteOffset);
			}

			Netcode::InputLayoutRef inputLayout = inputLayoutBuilder->Build();

			Netcode::Graphics::UploadBatch batch;

			auto appMesh = std::make_shared<Mesh>();

			uint8_t * const vBasePtr = reinterpret_cast<uint8_t *>(mesh->vertices);
			uint8_t * const iBasePtr = reinterpret_cast<uint8_t *>(mesh->indices);

			for(unsigned int i = 0; i < mesh->lodLevelsLength; ++i) {
				uint8_t * vData = vBasePtr + mesh->lodLevels[i].vertexBufferByteOffset;
				uint8_t * iData = nullptr;
				GpuResourceRef vbuffer = graphics->resources->CreateVertexBuffer(mesh->lodLevels[i].vertexBufferSizeInBytes, mesh->vertexSize, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
				uint64_t vCount = mesh->lodLevels[i].vertexCount;
				GpuResourceRef ibuffer = 0;
				uint64_t iCount = 0;

				batch.Upload(vbuffer, vData, mesh->lodLevels[i].vertexBufferSizeInBytes);
				batch.ResourceBarrier(vbuffer, ResourceState::COPY_DEST, ResourceState::VERTEX_AND_CONSTANT_BUFFER);

				if(mesh->indices != nullptr) {
					ibuffer = graphics->resources->CreateIndexBuffer(mesh->lodLevels[i].indexBufferSizeInBytes, DXGI_FORMAT_R32_UINT, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
					iData = iBasePtr + mesh->lodLevels[i].indexBufferByteOffset;
					iCount = mesh->lodLevels[i].indexCount;

					batch.Upload(ibuffer, iData, mesh->lodLevels[i].indexBufferSizeInBytes);
					batch.ResourceBarrier(ibuffer, ResourceState::COPY_DEST, ResourceState::INDEX_BUFFER);
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

			auto material = std::make_shared<TestMaterial>();

			material->data.diffuseColor = Netcode::Float4{ mat->diffuseColor.x, mat->diffuseColor.y,mat->diffuseColor.z, 1.0f };
			material->data.fresnelR0 = Netcode::Float3{ 0.05f, 0.05f, 0.05f };
			material->data.shininess = mat->shininess;

			graphics->frame->SyncUpload(batch);

			modelComponent->AddShadedMesh(appMesh, material);
		}

	}

public:

	virtual void OnResized(int w, int h) override {
		float asp = graphics->GetAspectRatio();
		gameScene->GetCamera()->GetComponent<Camera>()->aspect = asp;
		pageManager.WindowResized(Netcode::UInt2{ static_cast<uint32_t>(w), static_cast<uint32_t>(h) });
		renderSystem.renderer.OnResize(w, h);
	}

	virtual void AddAppEventHandlers(Netcode::Module::AppEventSystem * eventSystem) override {
		Netcode::Module::AApp::AddAppEventHandlers(eventSystem);

		eventSystem->AddHandler(this);
	}

	/*
	Initialize modules
	*/
	virtual void Setup(Netcode::Module::IModuleFactory * factory) override {
		events = std::make_unique<Netcode::Module::AppEventSystem>();

		window = factory->CreateWindowModule(this, 0);
		graphics = factory->CreateGraphicsModule(this, 0);
		audio = factory->CreateAudioModule(this, 0);
		network = factory->CreateNetworkModule(this, 0);

		StartModule(window.get());
		StartModule(graphics.get());
		StartModule(audio.get());
		StartModule(network.get());

		if(window) {
			window->ShowWindow();
		}

		AddAppEventHandlers(events.get());

		stopwatch.Start();

		px.CreateResources();

		LoadServices();
		LoadSystems();
		LoadAssets();
	}

	/*
	Advance simulation, update modules
	*/
	virtual void Run() override {
		while(window->KeepRunning()) {
			window->ProcessMessages();
			
			events->Dispatch();

			float dt = stopwatch.Restart();
			Simulate(dt);

			Render();

			window->CompleteFrame();
		}
	}

	/*
	Properly shutdown the application
	*/
	virtual void Exit() override {
		defaultPhysxMaterial.Reset();
		renderSystem.renderer.ui_Input.reset();
		pageManager.Destruct();
		Service::Clear();
		px.ReleaseResources();
		ShutdownModule(network.get());
		ShutdownModule(audio.get());
		ShutdownModule(graphics.get());
		ShutdownModule(window.get());
	}
};

