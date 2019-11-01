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

class EggApp : public Egg::SimpleApp {
protected:

	std::unique_ptr<Egg::Scene> scene;
	std::unique_ptr<Egg::Graphics::ResourceManager> resourceManager;
	Egg::ConstantBuffer<PerFrameCb> perFrameCb;
	std::unique_ptr<Egg::DebugPhysx> debugPhysx;
	Egg::CharacterController chCtrl;
	Egg::Camera::BaseCamera baseCam;
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
		//chCtrl.Update(dt);
		//const auto & chPos = chCtrl.GetPosition();

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
				
				model->multiMesh.Draw(commandList.Get());
				
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


		Egg::Asset::Model ybotModel;
		//Egg::Asset::Model railgun;

		Egg::Importer::ImportModel(L"ybot.eggasset", ybotModel);
		//Egg::Importer::ImportModel(L"railgun.eggasset", railgun);

		auto* playerObject = scene->New();
		playerObject->AddComponent<Egg::Transform>();
		auto * modelComponent = playerObject->AddComponent<Egg::Model>();
		modelComponent->multiMesh = resourceManager->LoadAssets(&ybotModel);

		
		//resourceManager->LoadAssets(&railgun);



		debugPhysx.reset(new Egg::DebugPhysx{});
		debugPhysx->CreateResources(device.Get());

		//debugPhysx->AddActor(pxSys.groundPlane)
		//	.SetOffset(0, DirectX::XMMatrixScaling(2000.0f, 2000.0f, 2000.0f));


		UploadResources();


	}

};
