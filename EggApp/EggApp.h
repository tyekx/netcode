#pragma once

#include <Egg/App.h>
#include <Egg/Importer.h>
#include <Egg/ConstantBuffer.hpp>
#include <Egg/Camera/Camera.h>
#include <Egg/PhysxSystem.h>
#include <Egg/Scene.h>
#include <Egg/Input.h>
#include <Egg/BasicGeometry.h>
#include <Egg/DebugPhysx.h>
#include <Egg/EggMath.h>
#include <Egg/Modules.h>

#include "Asset.h"


class GameApp : public Egg::Module::AApp {
	Egg::Stopwatch stopwatch;

	//@TODO: refactor this
	Egg::Asset::Model ybotModel;
	Model model;

	void Render() {
		graphics->Prepare();
		graphics->SetRenderTarget();
		graphics->ClearRenderTarget();

		/*
		foreach gameobject draw
		*/
		for(unsigned int i = 0; i < model.meshesLength; ++i) {
			graphics->Record(model.meshes[i].mesh);
		}

		graphics->Render();
		graphics->Present();
	}

	void Simulate(float dt) {
		//physics->Simulate(dt);

		/*
		foreach gameobject update
		*/
	}

	void LoadAssets() {

		Egg::Importer::ImportModel(L"ybot.eggasset", ybotModel);
		LoadItem(graphics.get(), &ybotModel, &model);

	}

public:
	/*
	Initialize modules
	*/
	virtual void Setup(Egg::Module::IModuleFactory * factory) override {
		window = factory->CreateWindowModule(this, 0);
		graphics = factory->CreateGraphicsModule(this, 0);
		audio = factory->CreateAudioModule(this, 0);
		physics = factory->CreatePhysicsModule(this, 0);
		network = nullptr;

		StartModule(window.get());
		StartModule(graphics.get());
		StartModule(physics.get());
		StartModule(audio.get());

		stopwatch.Start();

		LoadAssets();
	}

	/*
	Advance simulation, update modules
	*/
	virtual void Run() override {
		while(window->KeepRunning()) {
			window->ProcessMessages();

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

		float devCamX = Egg::Input::GetAxis("DevCameraX");
		float devCamZ = Egg::Input::GetAxis("DevCameraZ");
		float devCamY = Egg::Input::GetAxis("DevCameraY");

		DirectX::XMINT2 mouseDelta = Egg::Input::GetMouseDelta();

		DirectX::XMFLOAT2A normalizedMouseDelta{ -(float)(mouseDelta.x), -(float)(mouseDelta.y) };

		cameraPitch += mouseSpeed * normalizedMouseDelta.y * dt;
		cameraPitch = std::clamp(cameraPitch, -(DirectX::XM_PIDIV2 - 0.00001f), (DirectX::XM_PIDIV2 - 0.00001f));
		cameraYaw += mouseSpeed * normalizedMouseDelta.x * dt;

		if(cameraYaw < (-DirectX::XM_PI)) {
			cameraYaw += DirectX::XM_2PI;
		}

		if(cameraYaw > (DirectX::XM_PI)) {
			cameraYaw -= DirectX::XM_2PI;
		}

		DirectX::XMVECTOR cameraYawQuat = DirectX::XMQuaternionRotationRollPitchYaw(0.0f, cameraYaw, 0.0f);

		DirectX::XMFLOAT3A devCam = { devCamX, devCamY, devCamZ };
		DirectX::XMVECTOR devCamVec = DirectX::XMLoadFloat3A(&devCam);

		devCamVec = DirectX::XMVector3Rotate(devCamVec, cameraYawQuat);

		DirectX::XMVECTOR devCamPos = DirectX::XMLoadFloat3(&baseCam.Position);
		devCamVec = DirectX::XMVectorScale(devCamVec, speed * dt);
		devCamPos = DirectX::XMVectorAdd(devCamVec, devCamPos);

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

		DirectX::XMMATRIX vp = DirectX::XMMatrixMultiply(view, proj);

		perFrameCb->eyePos = DirectX::XMFLOAT3A{ baseCam.Position.x, baseCam.Position.y, baseCam.Position.z };
		perFrameCb->Light.position = DirectX::XMFLOAT4A{ 1.0f, 0.0f, 0.0f, 0.0f };
		perFrameCb->Light.intensity = DirectX::XMFLOAT3A{ 1.0f, 1.0f, 1.0f };
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

		animComponent->blackBoard.CreateResources(&ybotModel, modelComponent->boneDataCb, ybotModel.animationsLength, {
					{ "Idle",			4,		Egg::Animation::StateBehaviour::LOOP },
					{ "Forward",		6,		Egg::Animation::StateBehaviour::LOOP },
					{ "Backward",		7,		Egg::Animation::StateBehaviour::LOOP },
					{ "Left",			12,		Egg::Animation::StateBehaviour::LOOP },
					{ "Right",			13,		Egg::Animation::StateBehaviour::LOOP },
					{ "ForwardLeft",	10,		Egg::Animation::StateBehaviour::LOOP },
					{ "ForwardRight",	11,		Egg::Animation::StateBehaviour::LOOP },
					{ "BackwardLeft",	8,		Egg::Animation::StateBehaviour::LOOP },
					{ "BackwardRight",	9,		Egg::Animation::StateBehaviour::LOOP },
					{ "JumpStart",		5,		Egg::Animation::StateBehaviour::ONCE },
					{ "JumpLoop",		3,		Egg::Animation::StateBehaviour::LOOP },
					{ "JumpLand",		2,		Egg::Animation::StateBehaviour::ONCE }
				},
			   {
					{ "Forward",   "Idle",			&Egg::MovementController::IsIdle,					nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Forward",   "Backward",		&Egg::MovementController::IsMovingBackward,			nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Forward",   "Left",			&Egg::MovementController::IsMovingLeft,				nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Forward",   "Right",			&Egg::MovementController::IsMovingRight,			nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Forward",   "ForwardLeft",	&Egg::MovementController::IsMovingForwardLeft,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Forward",   "ForwardRight",	&Egg::MovementController::IsMovingForwardRight,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Forward",   "BackwardLeft",	&Egg::MovementController::IsMovingBackwardLeft,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Forward",   "BackwardRight",	&Egg::MovementController::IsMovingBackwardRight,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Forward",   "JumpStart",		&Egg::MovementController::IsJumping,				nullptr,							Egg::Animation::TransitionBehaviour::STOP_AND_LERP },

					{ "Backward",   "Idle",			&Egg::MovementController::IsIdle,					nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Backward",   "Forward",		&Egg::MovementController::IsMovingForward,			nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Backward",   "Left",			&Egg::MovementController::IsMovingLeft,				nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Backward",   "Right",		&Egg::MovementController::IsMovingRight,			nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Backward",   "ForwardLeft",	&Egg::MovementController::IsMovingForwardLeft,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Backward",   "ForwardRight",	&Egg::MovementController::IsMovingForwardRight,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Backward",   "BackwardLeft",	&Egg::MovementController::IsMovingBackwardLeft,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Backward",   "BackwardRight",&Egg::MovementController::IsMovingBackwardRight,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Backward",   "JumpStart",	&Egg::MovementController::IsJumping,				nullptr,							Egg::Animation::TransitionBehaviour::STOP_AND_LERP },

					{ "Left",   "Idle",				&Egg::MovementController::IsIdle,					nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Left",   "Forward",			&Egg::MovementController::IsMovingForward,			nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Left",   "Backward",			&Egg::MovementController::IsMovingBackward,			nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Left",   "Right",			&Egg::MovementController::IsMovingRight,			nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Left",   "ForwardLeft",		&Egg::MovementController::IsMovingForwardLeft,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Left",   "ForwardRight",		&Egg::MovementController::IsMovingForwardRight,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Left",   "BackwardLeft",		&Egg::MovementController::IsMovingBackwardLeft,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Left",   "BackwardRight",	&Egg::MovementController::IsMovingBackwardRight,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Left",   "JumpStart",		&Egg::MovementController::IsJumping,				nullptr,							Egg::Animation::TransitionBehaviour::STOP_AND_LERP },

					{ "Right",   "Idle",			&Egg::MovementController::IsIdle,					nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Right",   "Forward",			&Egg::MovementController::IsMovingForward,			nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Right",   "Backward",		&Egg::MovementController::IsMovingBackward,			nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Right",   "Left",			&Egg::MovementController::IsMovingLeft,				nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Right",   "ForwardLeft",		&Egg::MovementController::IsMovingForwardLeft,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Right",   "ForwardRight",	&Egg::MovementController::IsMovingForwardRight,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Right",   "BackwardLeft",	&Egg::MovementController::IsMovingBackwardLeft,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Right",   "BackwardRight",	&Egg::MovementController::IsMovingBackwardRight,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Right",   "JumpStart",		&Egg::MovementController::IsJumping,				nullptr,							Egg::Animation::TransitionBehaviour::STOP_AND_LERP },

					{ "ForwardLeft",   "Idle",			&Egg::MovementController::IsIdle,				nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardLeft",   "Forward",		&Egg::MovementController::IsMovingForward,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardLeft",   "Backward",		&Egg::MovementController::IsMovingBackward,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardLeft",   "Left",			&Egg::MovementController::IsMovingLeft,			nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardLeft",   "Right",			&Egg::MovementController::IsMovingRight,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardLeft",   "ForwardRight",	&Egg::MovementController::IsMovingForwardRight,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardLeft",   "BackwardLeft",	&Egg::MovementController::IsMovingBackwardLeft,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardLeft",   "BackwardRight",	&Egg::MovementController::IsMovingBackwardRight,nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardLeft",   "JumpStart",		&Egg::MovementController::IsJumping,			nullptr,							Egg::Animation::TransitionBehaviour::STOP_AND_LERP },

					{ "ForwardRight",   "Idle",			&Egg::MovementController::IsIdle,				nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardRight",   "Forward",		&Egg::MovementController::IsMovingForward,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardRight",   "Backward",		&Egg::MovementController::IsMovingBackward,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardRight",   "Left",			&Egg::MovementController::IsMovingLeft,			nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardRight",   "Right",		&Egg::MovementController::IsMovingRight,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardRight",   "ForwardLeft",	&Egg::MovementController::IsMovingForwardLeft,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardRight",   "BackwardLeft",	&Egg::MovementController::IsMovingBackwardLeft,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardRight",   "BackwardRight",&Egg::MovementController::IsMovingBackwardRight,nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "ForwardRight",   "JumpStart",	&Egg::MovementController::IsJumping,			nullptr,							Egg::Animation::TransitionBehaviour::STOP_AND_LERP },

					{ "BackwardLeft",   "Idle",			&Egg::MovementController::IsIdle,				nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardLeft",   "Forward",		&Egg::MovementController::IsMovingForward,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardLeft",   "Backward",		&Egg::MovementController::IsMovingBackward,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardLeft",   "Left",			&Egg::MovementController::IsMovingLeft,			nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardLeft",   "Right",		&Egg::MovementController::IsMovingRight,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardLeft",   "ForwardLeft",	&Egg::MovementController::IsMovingForwardLeft,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardLeft",   "ForwardRight",	&Egg::MovementController::IsMovingForwardRight,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardLeft",   "BackwardRight",&Egg::MovementController::IsMovingBackwardRight,nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardLeft",   "JumpStart",	&Egg::MovementController::IsJumping,			nullptr,							Egg::Animation::TransitionBehaviour::STOP_AND_LERP },

					{ "BackwardRight",   "Idle",		&Egg::MovementController::IsIdle,				nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardRight",   "Forward",		&Egg::MovementController::IsMovingForward,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardRight",   "Backward",	&Egg::MovementController::IsMovingBackward,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardRight",   "Left",		&Egg::MovementController::IsMovingLeft,			nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardRight",   "Right",		&Egg::MovementController::IsMovingRight,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardRight",   "ForwardLeft",	&Egg::MovementController::IsMovingForwardLeft,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardRight",   "ForwardRight",&Egg::MovementController::IsMovingForwardRight,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardRight",   "BackwardLeft",&Egg::MovementController::IsMovingBackwardLeft,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "BackwardRight",   "JumpStart",	&Egg::MovementController::IsJumping,			nullptr,							Egg::Animation::TransitionBehaviour::STOP_AND_LERP },

					{ "Idle",	   "Forward",			&Egg::MovementController::IsMovingForward,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Idle",	   "Backward",			&Egg::MovementController::IsMovingBackward,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Idle",	   "Left",				&Egg::MovementController::IsMovingLeft,			nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Idle",	   "Right",				&Egg::MovementController::IsMovingRight,		nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Idle",	   "ForwardLeft",		&Egg::MovementController::IsMovingForwardLeft,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Idle",	   "ForwardRight",		&Egg::MovementController::IsMovingForwardRight,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Idle",	   "BackwardLeft",		&Egg::MovementController::IsMovingBackwardLeft,	nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Idle",	   "BackwardRight",		&Egg::MovementController::IsMovingBackwardRight,nullptr,							Egg::Animation::TransitionBehaviour::LERP },
					{ "Idle",	   "JumpStart",			&Egg::MovementController::IsJumping,			nullptr,							Egg::Animation::TransitionBehaviour::STOP_AND_LERP },
					 
					{ "JumpStart", "JumpLoop",	&Egg::MovementController::IsJumping,		&Egg::Animation::AnimationState::IsFinished,	Egg::Animation::TransitionBehaviour::STOP_AND_LERP },
					{ "JumpStart", "JumpLand",	&Egg::MovementController::IsOnGround, 		nullptr,										Egg::Animation::TransitionBehaviour::STOP_AND_LERP },
					{ "JumpLoop",  "JumpLand",	&Egg::MovementController::IsOnGround,		nullptr,										Egg::Animation::TransitionBehaviour::STOP_AND_LERP },
					{ "JumpLand",  "Idle",		nullptr,									&Egg::Animation::AnimationState::IsFinished,	Egg::Animation::TransitionBehaviour::LERP },
			   });




		//debugPhysx.reset(new Egg::DebugPhysx{});
		//debugPhysx->CreateResources(device.Get(), resourceManager.get());

		//debugPhysx->AddActor(pxSys.groundPlane).SetOffset(0, DirectX::XMMatrixTranspose(DirectX::XMMatrixScaling(2000.0f, 2000.0f, 2000.0f)));


	}

};
*/