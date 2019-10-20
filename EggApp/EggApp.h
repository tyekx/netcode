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
	Egg::ConstantBufferVector<PerMeshCb> meshesCb;
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

	Egg::Mesh::MultiMesh::P railgunMesh;
	Egg::Asset::Model railgun;
	Egg::ConstantBuffer<PerObjectCb> perObjCb;
	Egg::ConstantBuffer<BoneDataCb> railgunBoneCb;

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

		DirectX::XMMATRIX rightHandTransform = DirectX::XMLoadFloat4x4A(&chCtrl.boneDataCb->ToRootTransform[28]);
		DirectX::XMMATRIX parentTransform = DirectX::XMLoadFloat4x4A(&chCtrl.cb->Model);
		DirectX::XMMATRIX scaling = DirectX::XMMatrixScaling(0.7f, 0.7f, 0.7f);
		DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(0.1f, -0.3f, -DirectX::XM_PIDIV2 - 0.07f);
		DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(-11.0f, -3.0f, 2.0f);
		DirectX::XMMATRIX localTransform = DirectX::XMMatrixMultiply(scaling, rotation);

		localTransform = DirectX::XMMatrixMultiply(localTransform, translation);
		localTransform = DirectX::XMMatrixMultiply(localTransform, rightHandTransform);
		localTransform = DirectX::XMMatrixMultiply(localTransform, DirectX::XMMatrixTranspose(parentTransform));
		DirectX::XMStoreFloat4x4A(&perObjCb->Model, DirectX::XMMatrixTranspose(localTransform));
	//	perObjCb->InvModel = identity;
		perObjCb.Upload();

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

		for(auto & rim : railgunMesh->GetMeshes()) {
			auto mat = rim->GetMaterial();
			auto geom = rim->GetGeometry();

			mat->ApplyPipelineState(commandList.Get());
			mat->BindConstantBuffer(commandList.Get(), perObjCb);
			mat->BindConstantBuffer(commandList.Get(), perFrameCb);
			mat->BindConstantBuffer(commandList.Get(), railgunBoneCb);
			mat->BindConstantBuffer(commandList.Get(), PerMeshCb::id, meshesCb.TranslateAddr(rim->GetMeshData()));

			geom->Draw(commandList.Get());
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
		cb.CreateResources(device.Get());
		perFrameCb.CreateResources(device.Get());
		railgunBoneCb.CreateResources(device.Get());
		perObjCb.CreateResources(device.Get());
		meshesCb.CreateResources(device.Get());



		DirectX::XMMATRIX m = DirectX::XMMatrixIdentity();
		DirectX::XMFLOAT4X4A identity;
		DirectX::XMStoreFloat4x4A(&identity, m);
		for(int i = 0; i < 128; ++i) {
			railgunBoneCb->BindTransform[i] = identity;
		}
		railgunBoneCb.Upload();

		DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(-DirectX::XM_PIDIV2, 0.0f, 0.0f);

		DirectX::XMStoreFloat4x4A(&perObjCb->Model, DirectX::XMMatrixTranspose(rotation));
		perObjCb->InvModel = identity;
		perObjCb.Upload();

		Egg::Asset::Model ybotModel;
		Egg::Importer::ImportModel(L"ybot.eggasset", ybotModel);
		
		Egg::Importer::ImportModel(L"railgun.eggasset", railgun);

		com_ptr<ID3DBlob> avatarVS = Egg::Shader::LoadCso(L"AvatarVS.cso");
		com_ptr<ID3DBlob> avatarPS = Egg::Shader::LoadCso(L"AvatarPS.cso");
		com_ptr<ID3D12RootSignature> rootSig = Egg::Shader::LoadRootSignature(device.Get(), avatarVS.Get());

		railgunMesh = Egg::Mesh::MultiMesh::Create();

		Egg::PipelineState::P pso = Egg::PipelineState::Create();
		pso->SetRootSignature(rootSig);
		pso->SetVertexShader(avatarVS);
		pso->SetPixelShader(avatarPS);
		pso->SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
		pso->SetDSVFormat(DXGI_FORMAT_D32_FLOAT);

		for(unsigned int i = 0; i < railgun.meshesLength; ++i) {

			Egg::Mesh::Geometry::P geom = Egg::Mesh::IndexedGeometry::Create(device.Get(),
																			 railgun.meshes[i].vertices, railgun.meshes[i].verticesLength, railgun.meshes[i].vertexSize,
																			 railgun.meshes[i].indices, railgun.meshes[i].indicesLength * (UINT32)sizeof(unsigned int));
			geom->SetVertexType(railgun.meshes[i].vertexType);
			Egg::Material::P mat = Egg::Material::Create(psoManager.get(), geom, pso);

			mat->ConstantBufferSlot(0, PerMeshCb::id);
			mat->ConstantBufferSlot(1, PerObjectCb::id);
			mat->ConstantBufferSlot(2, BoneDataCb::id);
			mat->ConstantBufferSlot(3, PerFrameCb::id);

			PerMeshCb * meshData = meshesCb.Next();
			meshData->diffuseColor = DirectX::XMFLOAT4A{ railgun.materials[i].diffuseColor.x, railgun.materials[i].diffuseColor.y, railgun.materials[i].diffuseColor.z, 1.0f };
			meshData->fresnelR0 = DirectX::XMFLOAT3{ 0.05f, 0.05f, 0.05f };
			meshData->shininess = 2.0f;
			railgunMesh->Add(geom, mat, meshData);
		}


		debugPhysx.reset(new Egg::DebugPhysx{});
		debugPhysx->CreateResources(device.Get(), psoManager.get());

		debugPhysx->AddActor(pxSys.groundPlane)
			.SetOffset(0, DirectX::XMMatrixScaling(2000.0f, 2000.0f, 2000.0f));

		chCtrl.SetCharacterModel(std::move(ybotModel));
		chCtrl.CreateResources(device.Get(), psoManager.get(), debugPhysx.get(), pxSys.controllerManager, meshesCb);

		meshesCb.Upload();


	}

};
