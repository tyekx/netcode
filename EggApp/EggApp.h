#pragma once

#include <Egg/Input.h>
#include <Egg/BasicGeometry.h>
#include <Egg/DebugPhysx.h>
#include <Egg/EggMath.h>
#include <Egg/Modules.h>
#include <Egg/Stopwatch.h>
#include <Egg/Service.hpp>

#include "Asset.h"
#include "GameObject.h"
#include "Systems.h"
#include "Scene.h"
#include "DevCameraScript.h"
#include "PlayerBehavior.h"
#include "Snippets.h"
#include "PhysxHelpers.h"
#include "AssetManager.h"

using Egg::Graphics::ResourceType;
using Egg::Graphics::ResourceState;

class GameApp : public Egg::Module::AApp, Egg::Module::TAppEventHandler {
	Egg::Stopwatch stopwatch;
	Egg::Physics::PhysXScene pxScene;
	physx::PxMaterial * defaultPhysxMaterial;
	AssetManager assetManager;
	
	Egg::MovementController movCtrl;

	TransformSystem transformSystem;
	ScriptSystem scriptSystem;
	RenderSystem renderSystem;
	AnimationSystem animSystem;
	PhysXSystem pxSystem;

	Scene scene;

	float totalTime;

	void Render() {
		graphics->frame->Prepare();

		//scene.UpdatePerFrameCb();
		
		//renderSystem.renderer.perFrameData = &scene.perFrameData;
		//renderSystem.renderer.ssaoData = &scene.ssaoData;

		FrameGraphBuilder builder;

		/*
		foreach gameobject draw
		
		for(std::size_t i = 0; i < scene.count; ++i) {
			GameObject * obj = scene.objects.data() + i;
			if(obj->IsActive()) {
				transformSystem.Run(obj);
				pxSystem.Run(obj);
				renderSystem.Run(obj);
			}
		}
	*/
		renderSystem.renderer.CreateFrameGraph(builder);

		FrameGraph graph = builder.Build(graphics->resources);
		graph.Render(graphics->renderer);

		graphics->frame->Render();
		graphics->frame->Present();
		
		renderSystem.renderer.Reset();
	}

	void Simulate(float dt) {
		totalTime += dt;
		pxScene.Simulate(dt);
		/*
		for(std::size_t i = 0; i < scene.count; ++i) {
			GameObject * obj = scene.objects.data() + i;
			if(obj->IsActive()) {
				scriptSystem.Run(obj, dt);
				animSystem.Run(obj, dt);
			}
		}*/

		//auto* cam = scene.camera->GetComponent<Camera>();
		//auto * t = scene.camera->GetComponent<Transform>();

		//auto xmpv = DirectX::XMLoadFloat3(&t->position);
		//auto xmav = DirectX::XMLoadFloat3(&cam->ahead);

		//xmpv = DirectX::XMVectorAdd(xmpv, DirectX::XMVectorScale(xmav, 10.0f));

		//DirectX::XMFLOAT3 p;
		//DirectX::XMStoreFloat3(&p, xmpv);

		//pxScene.UpdateDebugCamera(t->position, cam->up, p);
	}

	void LoadServices() {
		using Service = Egg::Service<std::tuple<AssetManager>>;

		Service::Init<AssetManager>();

		auto * assetManager = Service::Get<AssetManager>();

	}

	void LoadAssets() {
		//scene.Setup();

		Egg::Input::SetAxis("Vertical", 'W', 'S');
		Egg::Input::SetAxis("Horizontal", 'A', 'D');
		Egg::Input::SetAxis("Jump", VK_SPACE, 0);
		Egg::Input::SetAxis("Fire", VK_LBUTTON, 0);

		renderSystem.CreatePermanentResources(graphics.get());

		/*
		GameObject * avatar = scene.Insert();
		avatar->AddComponent<Transform>();
		Model * model = avatar->AddComponent<Model>();
		Animation * anim = avatar->AddComponent<Animation>();

		auto* physics = pxScene.Get();
		auto * pxController = pxScene.CreateController();
		auto * pxMaterial = physics->createMaterial(0.5f, 0.5f, 0.5f);

		defaultPhysxMaterial = physics->createMaterial(0.5f, 0.5f, 0.5f);

		physx::PxShape * defaultControllerCapsuleShape = nullptr;
		pxController->getActor()->getShapes(&defaultControllerCapsuleShape, 1, 0);
		defaultControllerCapsuleShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, false);

		physx::PxRigidDynamic * pxActor = physics->createRigidDynamic(physx::PxTransform{ ToPxVec3(pxController->getPosition()), physx::PxQuat(0.0f, 0.0f, 0.0f, 1.0f) });
		pxActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
		pxActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES, true);
		
		{
			auto planeActor = physx::PxCreatePlane(*physics, physx::PxPlane{ 0.0f, 1.0f, 0.0f, 0.0f }, *pxMaterial);
			pxScene.AddActor(planeActor);
		}
		std::vector<ColliderShape> localShapes;
		localShapes.reserve(ybotModel.colliders.Size());

		for(size_t colliderI = 0; colliderI < ybotModel.colliders.Size(); ++colliderI) {
			localShapes.push_back(ybotModel.colliders[colliderI]);
			auto * shape = CreateHitboxShapeFromAsset(ybotModel.colliders[colliderI], physics);
			pxActor->attachShape(*shape);
		}

		Collider * coll = avatar->AddComponent<Collider>();
		coll->shapes = std::move(localShapes);
		coll->actorRef = pxActor;
		pxScene.AddActor(pxActor);

		CreateYbotAnimationComponent(&ybotModel, anim);
		animSystem.SetMovementController(&movCtrl);

		Camera * fpsCam = avatar->AddComponent<Camera>();
		fpsCam->ahead = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };
		fpsCam->aspect = graphics->GetAspectRatio();
		fpsCam->farPlane = 10000.0f;
		fpsCam->nearPlane = 1.0f;
		fpsCam->up = DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f };

		Script* s = avatar->AddComponent<Script>();
		auto playerBehaviour = std::make_unique<PlayerBehavior>();
		
		playerBehaviour->SetController(pxController);
		s->SetBehavior(std::move(playerBehaviour));
		s->Setup(avatar);

		scene.SetCamera(avatar);*/
	}

	void InitializeBoneData(Model* modelComponent) {
		const DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
		modelComponent->boneData = std::make_unique<BoneData>();
		for(unsigned int i = 0; i < BoneData::MAX_BONE_COUNT; ++i) {
			DirectX::XMStoreFloat4x4A(&modelComponent->boneData->BindTransform[i], identity);
			DirectX::XMStoreFloat4x4A(&modelComponent->boneData->ToRootTransform[i], identity);
		}
	}

	void LoadComponents(Egg::Asset::Model * model, GameObject * gameObject) {
		gameObject->AddComponent<Transform>();

		if(model->meshes.Size() > 0) {
			Model * modelComponent = gameObject->AddComponent<Model>();
			LoadModelComponent(model, modelComponent);

			if(model->bones.Size() > 0 && model->animations.Size() > 0) {
				InitializeBoneData(modelComponent);
			}
		}

		if(model->colliders.Size() > 0) {
			Collider * colliderComponent = gameObject->AddComponent<Collider>();
			LoadColliderComponent(model, colliderComponent);
		}

	}

	void LoadColliderComponent(Egg::Asset::Model * model, Collider * colliderComponent) {
		if(model->colliders.Size() == 0) {
			return;
		}

		physx::PxPhysics * px = pxScene.Get();
		physx::PxRigidDynamic * actor = px->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
		actor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
		std::vector<ColliderShape> eggShapes;
		eggShapes.reserve(model->colliders.Size());

		for(size_t i = 0; i < model->colliders.Size(); ++i) {
			Egg::Asset::Collider * eggCollider = model->colliders.Data() + i;
			eggShapes.push_back(*eggCollider);
			physx::PxShape * shape = nullptr;
			physx::PxShapeFlags shapeFlags;

			if(eggCollider->boneReference > 0 && eggCollider->boneReference < 0x7F) {
				shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eTRIGGER_SHAPE | physx::PxShapeFlag::eVISUALIZATION;
			} else {
				shapeFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION;
			}

			if(eggCollider->type == Egg::Asset::ColliderType::MESH) {
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
				pxScene.cooking->cookConvexMesh(cmd, buf, &r);

				physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
				physx::PxConvexMesh * convexMesh = px->createConvexMesh(input);

				physx::PxConvexMeshGeometry pcmg{ convexMesh };

				shape = px->createShape(pcmg, *defaultPhysxMaterial, true, shapeFlags);

			} else {
				shape = CreatePrimitiveShapeFromAsset(*eggCollider, px, defaultPhysxMaterial, shapeFlags);
			}

			actor->attachShape(*shape);
		}

		colliderComponent->actorRef = actor;
		colliderComponent->shapes = std::move(eggShapes);
	}

	void LoadModelComponent(Egg::Asset::Model * model, Model * modelComponent) {
		for(size_t meshIdx = 0; meshIdx < model->meshes.Size(); ++meshIdx) {
			Egg::Asset::Material * mat = model->materials.Data() + model->meshes[meshIdx].materialId;
			Egg::Asset::Mesh * mesh = model->meshes.Data() + meshIdx;

			DirectX::XMStoreFloat4x4A(&modelComponent->perObjectData.Model, DirectX::XMMatrixIdentity());
			DirectX::XMStoreFloat4x4A(&modelComponent->perObjectData.InvModel, DirectX::XMMatrixIdentity());

			Egg::InputLayoutBuilderRef inputLayoutBuilder = graphics->CreateInputLayoutBuilder();

			for(uint32_t ilIdx = 0; ilIdx < mesh->inputElementsLength; ++ilIdx) {
				inputLayoutBuilder->AddInputElement(mesh->inputElements[ilIdx].semanticName,
					mesh->inputElements[ilIdx].semanticIndex,
					mesh->inputElements[ilIdx].format,
					mesh->inputElements[ilIdx].byteOffset);
			}

			Egg::InputLayoutRef inputLayout = inputLayoutBuilder->Build();

			Egg::Graphics::UploadBatch batch;

			auto appMesh = std::make_shared<Mesh>();

			uint8_t * const vBasePtr = reinterpret_cast<uint8_t *>(mesh->vertices);
			uint8_t * const iBasePtr = reinterpret_cast<uint8_t *>(mesh->indices);

			for(unsigned int i = 0; i < mesh->lodLevelsLength; ++i) {
				uint8_t * vData = vBasePtr + mesh->lodLevels[i].vertexBufferByteOffset;
				uint8_t * iData = nullptr;
				uint64_t vbuffer = graphics->resources->CreateVertexBuffer(mesh->lodLevels[i].vertexBufferSizeInBytes, mesh->vertexSize, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST);
				uint64_t vCount = mesh->lodLevels[i].vertexCount;
				uint64_t ibuffer = 0;
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

			material->data.diffuseColor = DirectX::XMFLOAT4A{ mat->diffuseColor.x, mat->diffuseColor.y,mat->diffuseColor.z, 1.0f };
			material->data.fresnelR0 = DirectX::XMFLOAT3{ 0.05f, 0.05f, 0.05f };
			material->data.shininess = mat->shininess;

			graphics->frame->SyncUpload(batch);

			modelComponent->AddShadedMesh(appMesh, material);
		}

	}

public:

	virtual void OnResized(int w, int h) override {
		float asp = graphics->GetAspectRatio();
		//scene.camera->GetComponent<Camera>()->aspect = asp;
		renderSystem.renderer.OnResize(w, h);
	}

	virtual void AddAppEventHandlers(Egg::Module::AppEventSystem * eventSystem) override {
		Egg::Module::AApp::AddAppEventHandlers(eventSystem);

		eventSystem->AddHandler(this);
	}

	/*
	Initialize modules
	*/
	virtual void Setup(Egg::Module::IModuleFactory * factory) override {
		events = std::make_unique<Egg::Module::AppEventSystem>();

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

		pxScene.CreateResources();

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
		pxScene.ReleaseResources();
		ShutdownModule(network.get());
		ShutdownModule(audio.get());
		ShutdownModule(graphics.get());
		ShutdownModule(window.get());
	}
};

