#pragma once

#include <Egg/SimpleApp.h>
#include <Egg/Importer.h>
#include <Egg/ConstantBuffer.hpp>
#include <Egg/Camera/Camera.h>
#include <Egg/PhysxSystem.h>
#include <Egg/Scene.h>
#include <Egg/Input.h>
#include <Egg/BasicGeometry.h>
#include <Egg/DebugPhysx.h>
#include <Egg/CharacterController.h>
#include <Egg/EggMath.h>
#include <Egg/ResourceManager.h>

class AnimationSystem {


public:
	constexpr static SignatureType Required() {
		return (0x1ULL << TupleIndexOf<AnimationComponent, COMPONENTS_T>::value);
	}

	constexpr static SignatureType Incompatible() {
		return (0x0ULL);
	}

	void Run(Egg::GameObject * go, float dt, Egg::MovementController * movCtrl) {

		if((go->GetSignature() & Required()) != 0) {
			AnimationComponent * animComponent = go->GetComponent<AnimationComponent>();
			animComponent->blackBoard.Update(dt, movCtrl);
		}

	}

};

class EggApp : public Egg::SimpleApp {
protected:
	Egg::Asset::Model ybotModel;
	std::unique_ptr<Egg::Scene> scene;
	std::unique_ptr<Egg::Graphics::ResourceManager> resourceManager;
	Egg::ConstantBuffer<PerFrameCb> perFrameCb;
	std::unique_ptr<Egg::DebugPhysx> debugPhysx;
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

	EggApp() : Egg::SimpleApp{}, perFrameCb{}, debugPhysx{}, baseCam{}, pxSys{}, speed{}, mouseSpeed{}, animT{ 0.0f }, fireEnabled{ true } {
		cameraPitch = 0.0f;
		cameraYaw = 0.0f;
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


		movCtrl.Update();
		for(UINT i = 0; i < scene->GetObjectCount(); ++i) {
			auto * go = scene->operator[](i);
			animSys.Run(go, dt, &movCtrl);
		}

		//DirectX::XMVECTOR offsetedPos = DirectX::XMVectorAdd(devCamPos, LoadPxExtendedVec3(chPos) );

		pxSys.Simulate(dt);
		debugPhysx->AfterPhysxUpdate(dt);

		baseCam.UpdateMatrices();
		//DirectX::XMStoreFloat3(&baseCam.Position, devCamPos);

		DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4A(&baseCam.GetViewMatrix());
		DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4A(&baseCam.GetProjMatrix());

		DirectX::XMMATRIX vp = DirectX::XMMatrixMultiply(view, proj);

		perFrameCb->eyePos = DirectX::XMFLOAT3A{ baseCam.Position.x, baseCam.Position.y, baseCam.Position.z };
		perFrameCb->Light.position = DirectX::XMFLOAT4A{ 1.0f, 0.0f, 0.0f, 0.0f };
		perFrameCb->Light.intensity = DirectX::XMFLOAT3A{ 1.0f, 1.0f, 1.0f };
		DirectX::XMStoreFloat4x4A(&perFrameCb->ViewProj, DirectX::XMMatrixTranspose(vp));
		perFrameCb.Upload();

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

	virtual void PopulateCommandList() override {

		commandAllocator->Reset();
		commandList->Reset(commandAllocator.Get(), nullptr);

		commandList->RSSetViewports(1, &viewPort);
		commandList->RSSetScissorRects(1, &scissorRect);

		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

		CD3DX12_CPU_DESCRIPTOR_HANDLE rHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorHandleIncrementSize);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());
		commandList->OMSetRenderTargets(1, &rHandle, FALSE, &dsvHandle);

		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		commandList->ClearRenderTargetView(rHandle, clearColor, 0, nullptr);
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		//debugPhysx->Draw(commandList.Get(), perFrameCb);
		//chCtrl.Draw(commandList.Get(), perFrameCb);

		for(UINT i = 0; i < scene->GetObjectCount(); ++i) {
			auto * obj = scene->operator[](i);

			if(obj->HasComponent<Egg::Transform>() && obj->HasComponent<Egg::Model>()) {

				Egg::Transform * transform = obj->GetComponent<Egg::Transform>();
				Egg::Model * model = obj->GetComponent<Egg::Model>();

				DirectX::XMStoreFloat4x4A(&model->multiMesh.perObjectCb->Model, DirectX::XMMatrixIdentity());
				DirectX::XMStoreFloat4x4A(&model->multiMesh.perObjectCb->InvModel, DirectX::XMMatrixIdentity());

				model->multiMesh.Draw(commandList.Get(), perFrameCb.GetGPUVirtualAddress());

			}
		}


		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		DX_API("Failed to close command list")
			commandList->Close();
	}

	void CreateSwapChainResources() override {
		Egg::SimpleApp::CreateSwapChainResources();
		baseCam.Aspect = aspectRatio;
	}

	/*
	Almost a render call
	*/
	void UploadResources() {
		DX_API("Failed to reset command allocator (UploadResources)")
			commandAllocator->Reset();
		DX_API("Failed to reset command list (UploadResources)")
			commandList->Reset(commandAllocator.Get(), nullptr);

		resourceManager->UploadResources(commandList.Get());

		DX_API("Failed to close command list (UploadResources)")
			commandList->Close();

		ID3D12CommandList * commandLists[] = { commandList.Get() };
		commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		WaitForPreviousFrame();

		resourceManager->ReleaseUploadResources();
	}

	virtual void CreateResources() override {
		Egg::SimpleApp::CreateResources();

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
		Egg::SimpleApp::ReleaseResources();
	}

	virtual void LoadAssets() override {
		perFrameCb.CreateResources(device.Get());
		scene = std::make_unique<Egg::Scene>();
		resourceManager = std::make_unique<Egg::Graphics::ResourceManager>();
		resourceManager->CreateResources(device.Get());

		DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(-DirectX::XM_PIDIV2, 0.0f, 0.0f);


		//Egg::Asset::Model railgun;

		Egg::Importer::ImportModel(L"ybot.eggasset", ybotModel);
		//Egg::Importer::ImportModel(L"railgun.eggasset", railgun);

		auto * playerObject = scene->New();
		playerObject->AddComponent<Egg::Transform>();
		auto * modelComponent = playerObject->AddComponent<Egg::Model>();
		modelComponent->multiMesh = resourceManager->LoadAssets(&ybotModel);
		auto * animComponent = playerObject->AddComponent<AnimationComponent>();

		//@TODO: would be better to handle this inside the animation system
		animComponent->blackBoard.Bind(modelComponent->multiMesh.boneDataCb);
		animComponent->blackBoard.CreateResources(&ybotModel, ybotModel.animationsLength, {
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

		//resourceManager->LoadAssets(&railgun);



		debugPhysx.reset(new Egg::DebugPhysx{});
		debugPhysx->CreateResources(device.Get());

		//debugPhysx->AddActor(pxSys.groundPlane)
		//	.SetOffset(0, DirectX::XMMatrixScaling(2000.0f, 2000.0f, 2000.0f));


		UploadResources();


	}

};
