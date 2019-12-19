#pragma once

#include <Egg/Importer.h>
#include <Egg/Input.h>
#include <Egg/BasicGeometry.h>
#include <Egg/DebugPhysx.h>
#include <Egg/EggMath.h>
#include <Egg/Modules.h>
#include <Egg/Stopwatch.h>

#include "Asset.h"
#include "GameObject.h"
#include "Systems.h"
#include "Scene.h"
#include "DevCameraScript.h"
#include "Snippets.h"


class GameApp : public Egg::Module::AApp, Egg::Module::TAppEventHandler {
	Egg::Stopwatch stopwatch;

	//@TODO: refactor these
	Egg::Asset::Model ybotModel;
	Egg::MovementController movCtrl;

	PerFrameCb * perFrameCb;

	TransformSystem transformSystem;
	ScriptSystem scriptSystem;
	RenderSystem renderSystem;
	AnimationSystem animSystem;

	Scene scene;
	
	

	void Render() {
		graphics->Prepare();
		graphics->SetRenderTarget();
		graphics->ClearRenderTarget();

		

		scene.UpdatePerFrameCb(perFrameCb);

		/*
		foreach gameobject draw
		*/
		for(std::size_t i = 0; i < scene.count; ++i) {
			transformSystem.Run(scene.objects.data() + i);
			renderSystem.Run(scene.objects.data() + i);
		}

		graphics->Render();
		graphics->Present();
	}

	void Simulate(float dt) {
		physics->Simulate(dt);

		for(std::size_t i = 0; i < scene.count; ++i) {
			scriptSystem.Run(scene.objects.data() + i, dt);
			animSystem.Run(scene.objects.data() + 1, dt);
		}
	}

	void LoadAssets() {
		Egg::Input::SetAxis("Vertical", 'W', 'S');
		Egg::Input::SetAxis("Horizontal", 'A', 'D');
		Egg::Input::SetAxis("Jump", VK_SPACE, 0);
		Egg::Input::SetAxis("Fire", VK_LBUTTON, 0);

		{
			GameObject * camObj = scene.Insert();
			Transform *camT = camObj->AddComponent<Transform>();
			Script * scriptComponent = camObj->AddComponent<Script>();
			Camera * camComponent = camObj->AddComponent<Camera>();
			camT->position = DirectX::XMFLOAT3{ 0.0f, 0.0f, 100.0f };
			camComponent->ahead = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };
			camComponent->nearPlane = 1.0f;
			camComponent->farPlane = 10000.0f;
			camComponent->up = DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f };
			scriptComponent->SetBehavior(std::make_unique<DevCameraScript>());
			scriptComponent->Setup(camObj);
			scene.SetCamera(camObj);
		}


		GameObject * avatar = scene.Insert();
		avatar->AddComponent<Transform>();
		Model * model = avatar->AddComponent<Model>();
		Animation * anim = avatar->AddComponent<Animation>();

		{
			auto mat = physics->CreateMaterial(0.5f, 0.5f, 0.5f);
			auto planeActor = physics->CreatePlane(mat, DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f }, 0.0f);
			physics->AddToScene(planeActor);

			auto boxShape = physics->CreateBox(mat, DirectX::XMFLOAT3{ 100.0f, 50.0f, 10.0f });
			auto boxActor = physics->CreateDynamicActor(boxShape, 20.0f);
			physics->SetActorPosition(boxActor, DirectX::XMFLOAT3{ 0.0f, 500.0f, 0.0f });
			physics->SetActorRotation(boxActor, DirectX::XMFLOAT4{ sqrtf(2.0f) / 2.0f, 0.0f, -sqrtf(2.0f) / 2.0f, 0.0f });
			physics->AddToScene(boxActor);
		}

		Egg::HCBUFFER pfcb = graphics->AllocateCbuffer(sizeof(PerFrameCb));
		Egg::HCBUFFER pocb = graphics->AllocateCbuffer(sizeof(PerObjectCb));
		perFrameCb = reinterpret_cast<PerFrameCb *>(graphics->GetCbufferPointer(pfcb));
		model->perObjectCb = reinterpret_cast<PerObjectCb *>(graphics->GetCbufferPointer(pocb));

		perFrameCb->Lights[0] = Egg::DirectionalLight{ DirectX::XMFLOAT3A{ 1.0f, 1.0f, 1.0f}, DirectX::XMFLOAT4A{ 1.0f, 0.0f, 0.0f, 0.0f} };

		DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
		DirectX::XMStoreFloat4x4A(&model->perObjectCb->InvModel, identity);
		DirectX::XMStoreFloat4x4A(&model->perObjectCb->Model, identity);

		Egg::Importer::ImportModel(L"ybot.eggasset", ybotModel);

		Egg::HCBUFFER bdcb = UINT_MAX;
		if(ybotModel.animationsLength > 0) {
			bdcb = graphics->AllocateCbuffer(sizeof(BoneDataCb));
			model->boneDataCb = reinterpret_cast<BoneDataCb *>(graphics->GetCbufferPointer(bdcb));
		}


		for(unsigned int i = 0; i < ybotModel.bonesLength; ++i) {
			DirectX::XMStoreFloat4x4A(&model->boneDataCb->BindTransform[i], identity);
			DirectX::XMStoreFloat4x4A(&model->boneDataCb->ToRootTransform[i], identity);
		}

		LoadItem(graphics.get(), &ybotModel, model);

		for(auto & i : model->meshes) {
			Egg::HITEM item = i.mesh;
			graphics->AddCbuffer(item, pfcb, graphics->GetCbufferSlot(item, "PerFrameCb"));
			graphics->AddCbuffer(item, pocb, graphics->GetCbufferSlot(item, "PerObjectCb"));
			if(ybotModel.animationsLength > 0) {
				graphics->AddCbuffer(item, bdcb, graphics->GetCbufferSlot(item, "BoneDataCb"));
			}
		}

		CreateYbotAnimationComponent(&ybotModel, anim);
		animSystem.SetMovementController(&movCtrl);

		Script* s = avatar->AddComponent<Script>();
		s->SetBehavior(std::make_unique<PlayerBehavior>());
		s->Setup(avatar);
	}

public:

	virtual void OnResized(int w, int h) override {
		float asp = graphics->GetAspectRatio();
		scene.camera->GetComponent<Camera>()->aspect = asp;
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
		physics = factory->CreatePhysicsModule(this, 0);
		network = nullptr;

		StartModule(window.get());
		StartModule(graphics.get());
		StartModule(physics.get());
		StartModule(audio.get());

		if(window) {
			window->ShowWindow();
		}

		AddAppEventHandlers(events.get());

		stopwatch.Start();

		renderSystem.SetGraphics(graphics.get());
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
		ShutdownModule(network.get());
		ShutdownModule(physics.get());
		ShutdownModule(audio.get());
		ShutdownModule(graphics.get());
		ShutdownModule(window.get());
	}
};


/*
class EggApp : public Egg::App {
protected:


	Egg::Asset::Model ybotModel;
	Egg::Asset::Model railgun;
	std::unique_ptr<Egg::Scene> scene;
	std::unique_ptr<Egg::Graphics::IVisualEngine> graphicsEngine;
	PerFrameCb* perFrameCb;
	//std::unique_ptr<Egg::DebugPhysx> debugPhysx;
	Egg::Camera::BaseCamera baseCam;
	Egg::MovementController movCtrl;
	AnimationSystem animSys;
	Egg::PhysxSystem pxSys;
	DirectX::XMFLOAT3A velocity;
	float cameraPitch;
	float cameraYaw;
	float speed;
	float mouseSpeed;
	float animT;
	bool fireEnabled;
public:

	EggApp() : perFrameCb{}, baseCam{}, pxSys{}, speed{}, mouseSpeed{}, animT{ 0.0f }, fireEnabled{ true } {
		cameraPitch = 0.0f;
		cameraYaw = 0.0f;
	}

	virtual void Render() override {
		UINT64 signature;
		Egg::GameObject * gameObj;

		//@TODO: move this out
		UINT64 graphicsSig = (0x1ULL << TupleIndexOf<Egg::Transform, COMPONENTS_T>::value) |
							 (0x1ULL << TupleIndexOf<Egg::Model, COMPONENTS_T>::value);

		// cleaning up and prepraring for recording
		graphicsEngine->PreUpdate();

		// recording commands
		for(UINT i = 0; i < scene->GetObjectCount(); ++i) {
			gameObj = scene->operator[](i);
			signature = gameObj->GetSignature();

			if((signature & graphicsSig) == graphicsSig) {

				Egg::Model * model = gameObj->GetComponent<Egg::Model>();

				graphicsEngine->Render(model->gpuResourcesHandle);
			}
		}

		// actual render call
		graphicsEngine->PostUpdate();
	}


	virtual void SetWindow(void * hwnd) override {
		//@TODO: move this out
		graphicsEngine = std::make_unique<Egg::Graphics::DX12::Engine>();
		graphicsEngine->CreateResources(hwnd);
		perFrameCb = graphicsEngine->GetPerFrameBuffer();
	}

	virtual void Update(float dt, float T) override {

	

		DirectX::XMFLOAT3 minusUnitZ{ 0.0f, 0.0f, -1.0f };
		DirectX::XMVECTOR cameraQuat = DirectX::XMQuaternionRotationRollPitchYaw(cameraPitch, cameraYaw, 0.0f);
		DirectX::XMVECTOR aheadStart = DirectX::XMLoadFloat3(&minusUnitZ);
		DirectX::XMVECTOR camUp = DirectX::XMLoadFloat3(&baseCam.Up);
		DirectX::XMStoreFloat3(&baseCam.Ahead, DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(aheadStart, cameraQuat)));
		DirectX::XMStoreFloat3(&baseCam.Position, devCamPos);
		baseCam.UpdateMatrices();

		movCtrl.Update();

		

		for(UINT i = 0; i < scene->GetObjectCount(); ++i) {
			auto * go = scene->operator[](i);
			if(go->HasComponent<Egg::Transform>() && go->HasComponent<Egg::Model>()) {
				Egg::Transform * transform = go->GetComponent<Egg::Transform>();
				Egg::Model * model = go->GetComponent<Egg::Model>();

				DirectX::XMVECTOR translation = DirectX::XMLoadFloat4(&transform->Position);
				DirectX::XMVECTOR rotation = DirectX::XMLoadFloat4(&transform->Rotation);
				DirectX::XMVECTOR scaling = DirectX::XMLoadFloat3(&transform->Scale);

				DirectX::XMMATRIX modelMat = DirectX::XMMatrixAffineTransformation(scaling, DirectX::XMQuaternionIdentity(), rotation, translation);

				DirectX::XMVECTOR modelMatDet = DirectX::XMMatrixDeterminant(modelMat);

				DirectX::XMStoreFloat4x4A(&model->perObjectCb->Model, DirectX::XMMatrixTranspose(modelMat));
				DirectX::XMStoreFloat4x4A(&model->perObjectCb->InvModel, DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&modelMatDet, modelMat)));
			}
		}

		for(UINT i = 0; i < scene->GetObjectCount(); ++i) {
			auto * go = scene->operator[](i);
			animSys.Run(go, dt, &movCtrl);
		}

		//DirectX::XMVECTOR offsetedPos = DirectX::XMVectorAdd(devCamPos, LoadPxExtendedVec3(chPos) );

		pxSys.Simulate(dt);
		//debugPhysx->AfterPhysxUpdate(dt);

		DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4A(&baseCam.GetViewMatrix());
		DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4A(&baseCam.GetProjMatrix());


		perFrameCb->eyePos = DirectX::XMFLOAT3A{ baseCam.Position.x, baseCam.Position.y, baseCam.Position.z };
		perFrameCb->Light.position = DirectX::XMFLOAT4A{ 1.0f, 0.0f, 0.0f, 0.0f };
		perFrameCb->Light.intensity = DirectX::XMFLOAT3A{ 1.0f, 1.0f, 1.0f };
		DirectX::XMMATRIX vp = DirectX::XMMatrixMultiply(view, proj);
		DirectX::XMStoreFloat4x4A(&perFrameCb->ViewProj, DirectX::XMMatrixTranspose(vp));

		Egg::Input::Reset();
	}

	virtual void KeyPressed(uint32_t keyCode) override {
		Egg::Input::KeyPressed(keyCode);
	}

	virtual void KeyReleased(uint32_t keyCode) override {
		Egg::Input::KeyReleased(keyCode);
	}

	virtual void Blur() override {
		Egg::Input::Blur();
	}

	virtual void Focused() override {
		Egg::Input::Focused();
	}

	virtual void CreateResources() override {
		//Egg::SimpleApp::CreateResources();

		Egg::Input::SetAxis("Vertical", 'W', 'S');
		Egg::Input::SetAxis("Horizontal", 'A', 'D');
		Egg::Input::SetAxis("Jump", VK_SPACE, 0);

		Egg::Input::SetAxis("DevCameraX", VK_NUMPAD4, VK_NUMPAD6);
		Egg::Input::SetAxis("DevCameraZ", VK_NUMPAD8, VK_NUMPAD5);
		Egg::Input::SetAxis("DevCameraY", VK_NUMPAD7, VK_NUMPAD9);

		Egg::Input::SetAxis("Fire", VK_LBUTTON, 0);

		pxSys.CreateResources();

		speed = 250.0f;
		mouseSpeed = 0.20f;
		baseCam.NearPlane = 1.0f;
		baseCam.FarPlane = 10000.0f;
		baseCam.Ahead = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };
		baseCam.Position = DirectX::XMFLOAT3{ 0.0f, 0.0f, 180.0f };
	}

	virtual void ReleaseResources() override {
		//Egg::SimpleApp::ReleaseResources();
	}

	virtual void LoadAssets() override {
		scene = std::make_unique<Egg::Scene>();

		DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(-DirectX::XM_PIDIV2, 0.0f, 0.0f);


		//

		Egg::Importer::ImportModel(L"ybot.eggasset", ybotModel);
		Egg::Importer::ImportModel(L"railgun.eggasset", railgun);
		DirectX::XMVECTOR weaponQuatV = DirectX::XMQuaternionRotationRollPitchYaw(-DirectX::XM_PIDIV2, 0, 0);
		DirectX::XMFLOAT4 weaponQuat;
		DirectX::XMStoreFloat4(&weaponQuat, weaponQuatV);
		auto * gunObject = scene->New();

		gunObject->AddComponent<Egg::Transform>()->Rotation = weaponQuat;
		auto * modelComp = gunObject->AddComponent<Egg::Model>();
		graphicsEngine->LoadAssets(modelComp, &railgun);
		auto * animComp = gunObject->AddComponent<AnimationComponent>();
		animComp->blackBoard.CreateResources(&railgun, modelComp->boneDataCb, railgun.animationsLength, {
												{ "Idle", 1, Egg::Animation::StateBehaviour::LOOP },
											    { "Shoot", 0, Egg::Animation::StateBehaviour::ONCE }
											 }, {
												 { "Idle", "Shoot", &Egg::MovementController::IsFiring, nullptr, Egg::Animation::TransitionBehaviour::LERP },
												 { "Shoot", "Idle", nullptr, &Egg::Animation::AnimationState::IsFinished, Egg::Animation::TransitionBehaviour::LERP }
											 });

		auto * playerObject = scene->New();
		auto *tcomp = playerObject->AddComponent<Egg::Transform>();
		tcomp->Scale = DirectX::XMFLOAT3{ 1,1,1 };
		
		auto * modelComponent = playerObject->AddComponent<Egg::Model>();
		graphicsEngine->LoadAssets(modelComponent, &ybotModel);
		auto * animComponent = playerObject->AddComponent<AnimationComponent>();

		baseCam.SetAspect(graphicsEngine->GetAspectRatio());




		//debugPhysx.reset(new Egg::DebugPhysx{});
		//debugPhysx->CreateResources(device.Get(), resourceManager.get());

		//debugPhysx->AddActor(pxSys.groundPlane).SetOffset(0, DirectX::XMMatrixTranspose(DirectX::XMMatrixScaling(2000.0f, 2000.0f, 2000.0f)));


	}

};
*/