#pragma once

#include <Egg/SimpleApp.h>
#include <Egg/Importer.h>
#include <Egg/ConstantBuffer.hpp>
#include <Egg/Camera/Camera.h>
#include <Egg/PhysxSystem.h>
#include <Egg/Scene.h>
#include <Egg/Mesh/MultiMesh.h>
#include <Egg/Input.h>
#include <Egg/BasicGeometry.h>
#include <Egg/DebugPhysx.h>
#include <Egg/CharacterController.h>
#include <Egg/EggMath.h>

class EggApp : public Egg::SimpleApp {
protected:
	Egg::Mesh::MultiMesh::P multi;
	Egg::ConstantBuffer<PerObjectCb> cb;
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

	EggApp() : Egg::SimpleApp{}, multi{}, cb{}, perFrameCb{}, debugPhysx{}, baseCam{}, pxSys{}, speed{}, mouseSpeed{}, animT{ 0.0f }, fireEnabled{ true } {
		cameraPitch = 0.0f;
		cameraYaw = 0.0f;
	}

	virtual void Update(float dt, float T) override {
		chCtrl.Update(dt);
		const auto & chPos = chCtrl.GetPosition();

		float devCamX = Egg::Input::GetAxis("DevCameraX");
		float devCamZ = Egg::Input::GetAxis("DevCameraZ");
		float devCamY = Egg::Input::GetAxis("DevCameraY");

		DirectX::XMINT2 mouseDelta = Egg::Input::GetMouseDelta();
		DirectX::XMFLOAT2A windowSize{ viewPort.Width, viewPort.Height };

		DirectX::XMFLOAT2A normalizedMouseDelta{ -(float)(mouseDelta.x), -(float)mouseDelta.y };

		cameraPitch = mouseSpeed * normalizedMouseDelta.y * dt;
		cameraYaw = mouseSpeed * normalizedMouseDelta.x * dt;

		DirectX::XMVECTOR cameraYawQuat = DirectX::XMQuaternionRotationRollPitchYaw(0.0f, cameraYaw, 0.0f);


		DirectX::XMFLOAT3A devCam = { devCamX, devCamY, devCamZ };
		DirectX::XMVECTOR devCamVec = DirectX::XMLoadFloat3A(&devCam);

		devCamVec = DirectX::XMVector3Rotate(devCamVec, cameraYawQuat);

		DirectX::XMVECTOR devCamPos = DirectX::XMLoadFloat3(&baseCam.Position);
		devCamVec = DirectX::XMVectorScale(devCamVec, speed * dt);
		devCamPos = DirectX::XMVectorAdd(devCamVec, devCamPos);

		DirectX::XMVECTOR cameraQuat = DirectX::XMQuaternionRotationRollPitchYaw(cameraPitch, cameraYaw, 0.0f);
		DirectX::XMVECTOR aheadStart = DirectX::XMLoadFloat3(&baseCam.Ahead);
		DirectX::XMVECTOR camUp = DirectX::XMLoadFloat3(&baseCam.Up);
		DirectX::XMStoreFloat3(&baseCam.Ahead, DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(aheadStart, cameraQuat)));
		DirectX::XMStoreFloat3(&baseCam.Up, DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(camUp, cameraYawQuat)));
		DirectX::XMStoreFloat3(&baseCam.Position, devCamPos);

		if(Egg::Input::GetAxis("Fire") > 0.0f) {
			if(fireEnabled) {
				float len = 2000.0;
				physx::PxVec3 dir = ToPxVec3(baseCam.Ahead);
				physx::PxVec3 origin = ToPxVec3(baseCam.Position);
				physx::PxRaycastBuffer hit;
				physx::PxHitFlags hitFlags = physx::PxHitFlag::eDEFAULT;
				bool b = pxSys.scene->raycast(origin, dir, len, hit);

				if(b) {
					debugPhysx->AddRaycast(baseCam.Ahead, baseCam.Position, len);
				} else {
					debugPhysx->AddRaycast(baseCam.Ahead, baseCam.Position, len, DirectX::XMFLOAT3{ 1.0f, 0.0f, 0.0f });
				}


				fireEnabled = false;
			}
		} else {
			fireEnabled = true;
		}

		//DirectX::XMVECTOR offsetedPos = DirectX::XMVectorAdd(devCamPos, LoadPxExtendedVec3(chPos) );

		pxSys.Simulate(dt);
		debugPhysx->AfterPhysxUpdate(dt);

		baseCam.UpdateMatrices();
		//DirectX::XMStoreFloat3(&baseCam.Position, devCamPos);

		DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4A(&baseCam.GetViewMatrix());
		DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4A(&baseCam.GetProjMatrix());

		DirectX::XMMATRIX vp = DirectX::XMMatrixMultiply(view, proj);

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

	/*
	virtual void MouseMove(int x, int y) override {
		Egg::Input::MouseMove(DirectX::XMINT2{ x, y });
	}*/

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

		debugPhysx->Draw(commandList.Get(), perFrameCb);
		chCtrl.Draw(commandList.Get(), perFrameCb);
		



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

		DX_API("Failed to close command list (UploadResources)")
			commandList->Close();

		ID3D12CommandList * commandLists[] = { commandList.Get() };
		commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		WaitForPreviousFrame();
	}

	virtual void CreateResources() override {
		Egg::SimpleApp::CreateResources();

		Egg::Input::SetAxis("Vertical", 'W', 'S');
		Egg::Input::SetAxis("Horizontal", 'A', 'D');

		Egg::Input::SetAxis("DevCameraX", VK_NUMPAD4, VK_NUMPAD6);
		Egg::Input::SetAxis("DevCameraZ", VK_NUMPAD8, VK_NUMPAD5);
		Egg::Input::SetAxis("DevCameraY", VK_NUMPAD7, VK_NUMPAD9);

		Egg::Input::SetAxis("Fire", VK_LBUTTON, 0);

		pxSys.CreateResources();

		speed = 250.0f;
		mouseSpeed = 0.35f;
		baseCam.NearPlane = 1.0f;
		baseCam.FarPlane = 1000.0f;
		baseCam.Ahead = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };
		baseCam.Position = DirectX::XMFLOAT3{ 0.0f, 0.0f, 180.0f };
	}

	virtual void ReleaseResources() override {
		Egg::SimpleApp::ReleaseResources();
	}

	virtual void LoadAssets() override {
		cb.CreateResources(device.Get());
		perFrameCb.CreateResources(device.Get());


		Egg::Asset::Model ybotModel;
		Egg::Importer::ImportModel(L"ybot.eggasset", ybotModel);
		



		debugPhysx.reset(new Egg::DebugPhysx{});
		debugPhysx->CreateResources(device.Get(), psoManager.get());

		debugPhysx->AddActor(pxSys.groundPlane)
			.SetOffset(0, DirectX::XMMatrixScaling(2000.0f, 2000.0f, 2000.0f));

		chCtrl.SetCharacterModel(std::move(ybotModel));
		chCtrl.CreateResources(device.Get(), psoManager.get(), debugPhysx.get(), pxSys.controllerManager);




	}

};
