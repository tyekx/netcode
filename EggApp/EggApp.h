#pragma once

#include <Egg/SimpleApp.h>
#include <Egg/Importer.h>
#include <Egg/ConstantBuffer.hpp>
#include <Egg/Camera/Camera.h>
#include <Egg/PhysxSystem.h>
#include <Egg/Scene.h>
#include <Egg/Mesh/MultiMesh.h>
#include <Egg/Input.h>
#include <Egg/AnimationController.h>
#include <Egg/BasicGeometry.h>
#include <Egg/DebugPhysx.h>

class EggApp : public Egg::SimpleApp {
protected:
	Egg::Mesh::MultiMesh::P multi;
	Egg::Asset::Model ybotModel;
	Egg::ConstantBuffer<PerObjectCb> cb;
	Egg::ConstantBuffer<PerFrameCb> perFrameCb;
	Egg::ConstantBuffer<BoneDataCb> boneDataCb;
	std::unique_ptr<Egg::AnimationController> animCtrl;
	std::unique_ptr<Egg::DebugPhysx> debugCharacter;
	std::unique_ptr<Egg::DebugPhysx> debugGround;
	physx::PxController * controller;
	Egg::Camera::BaseCamera baseCam;
	Egg::PhysxSystem pxSys;
	Egg::Scene scene;
	DirectX::XMFLOAT3A velocity;
	float speed;
	float mouseSpeed;
	float animT;
	bool onGround;
public:

	EggApp() : Egg::SimpleApp{}, multi{}, ybotModel{}, cb{}, perFrameCb{}, boneDataCb{}, animCtrl{ nullptr }, debugCharacter{}, debugGround{}, controller{ nullptr }, baseCam{}, pxSys{}, scene{}, speed{}, mouseSpeed{}, animT{ 0.0f }, onGround{ false } { }
	

	virtual void Update(float dt, float T) override {

		float vertical = Egg::Input::GetAxis("Vertical");
		float horizontal =	Egg::Input::GetAxis("Horizontal");

		DirectX::XMFLOAT3A vertComponent{ 0.0f, 0.0f, vertical };
		DirectX::XMFLOAT3A horComponent{ horizontal, 0.0f, 0.0f };
		DirectX::XMFLOAT3A gravityComponent{ 0.0f, -9.81f, 0.0f };


		float devCamX = Egg::Input::GetAxis("DevCameraX");
		float devCamZ = Egg::Input::GetAxis("DevCameraZ");
		float devCamY = Egg::Input::GetAxis("DevCameraY");

		DirectX::XMFLOAT3A devCam = { devCamX, devCamY, devCamZ };
		DirectX::XMVECTOR devCamVec = DirectX::XMLoadFloat3A(&devCam);
		DirectX::XMVECTOR devCamPos = DirectX::XMLoadFloat3(&baseCam.Position);
		devCamVec = DirectX::XMVectorScale(devCamVec, speed * dt);
		devCamPos = DirectX::XMVectorAdd(devCamVec, devCamPos);
		DirectX::XMStoreFloat3(&baseCam.Position, devCamPos);

		DirectX::XMVECTOR dir = DirectX::XMVectorAdd(DirectX::XMLoadFloat3A(&vertComponent), DirectX::XMLoadFloat3A(&horComponent));
		if(!onGround) {
			dir = DirectX::XMVectorAdd(dir, DirectX::XMLoadFloat3A(&gravityComponent));
		}
		dir = DirectX::XMVector3Normalize(dir);
		dir = DirectX::XMVectorScale(dir, speed * dt);

		DirectX::XMFLOAT3A dirResult;
		DirectX::XMStoreFloat3A(&dirResult, dir);

		physx::PxControllerCollisionFlags result = controller->move(physx::PxVec3(dirResult.x, dirResult.y, dirResult.z), 0.0f, dt, physx::PxControllerFilters{});

		if(result.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN)) {
			onGround = true;
		}

		pxSys.Simulate(dt);
		debugCharacter->AfterPhysxUpdate();
		debugGround->AfterPhysxUpdate();

		auto pxV3 = controller->getPosition();
		


		DirectX::XMMATRIX Tr = DirectX::XMMatrixTranslation(pxV3.x, pxV3.y, pxV3.z);
		devCamVec = DirectX::XMVector3Transform(devCamPos, Tr);
		DirectX::XMStoreFloat3(&baseCam.Position, devCamVec);

		DirectX::XMMATRIX offset = DirectX::XMMatrixMultiply(DirectX::XMMatrixTranslation(0.0f, -100.0f, 0.0f), Tr);

		DirectX::XMStoreFloat4x4A(&cb->Model, DirectX::XMMatrixTranspose(offset));
		DirectX::XMStoreFloat4x4A(&cb->InvModel, DirectX::XMMatrixIdentity());
		cb.Upload();

		animCtrl->Animate(boneDataCb->BindTransform, dt);
		boneDataCb.Upload();


		baseCam.UpdateMatrices();
		DirectX::XMStoreFloat3(&baseCam.Position, devCamPos);

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

	virtual void MouseMove(int x, int y) override {
		Egg::Input::MouseMove(DirectX::XMINT2{ x, y });
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

		debugCharacter->Draw(commandList.Get(), perFrameCb);
		debugGround->Draw(commandList.Get(), perFrameCb);
		
		auto meshes = multi->GetMeshes();
		for(auto & i : meshes) {
			auto mat = i->GetMaterial();
			mat->ApplyPipelineState(commandList.Get());
			mat->BindConstantBuffer(commandList.Get(), perFrameCb);
			mat->BindConstantBuffer(commandList.Get(), boneDataCb);
			mat->BindConstantBuffer(commandList.Get(), cb);

			auto geom = i->GetGeometry();
			geom->Draw(commandList.Get());
		}

		//commandList->SetGraphicsRootDescriptorTable(2, srvHeap->GetGPUDescriptorHandleForHeapStart());

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

		pxSys.CreateResources();

		speed = 250.0f;
		mouseSpeed = 1.5f;
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
		
		boneDataCb.CreateResources(device.Get());

		com_ptr<ID3DBlob> avatarVS = Egg::Shader::LoadCso(L"AvatarVS.cso");
		com_ptr<ID3DBlob> avatarPS = Egg::Shader::LoadCso(L"AvatarPS.cso");
		com_ptr<ID3D12RootSignature> rootSig = Egg::Shader::LoadRootSignature(device.Get(), avatarVS.Get());

		Egg::Importer::ImportModel(L"ybot.eggasset", ybotModel);

		animCtrl.reset(new Egg::AnimationController{ ybotModel.animations, ybotModel.animationsLength,
													 ybotModel.bones, ybotModel.bonesLength });

		physx::PxCapsuleControllerDesc cd;
		cd.behaviorCallback = NULL;
		cd.climbingMode = physx::PxCapsuleClimbingMode::eEASY;
		cd.contactOffset = 0.1f;
		cd.density = 10.0f;
		cd.invisibleWallHeight = 0.0f;
		cd.material = pxSys.physics->createMaterial(0.5f, 0.6f, 0.6f);
		cd.position = physx::PxExtendedVec3{ 0.0f, 200.0f, 0.0f };
		cd.nonWalkableMode = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING;
		cd.registerDeletionListener = true;
		cd.reportCallback = NULL;
		cd.scaleCoeff = 0.8f;
		cd.slopeLimit = 0.7071f;
		cd.stepOffset = 5.0f;
		cd.upDirection = physx::PxVec3{ 0.0f, 1.0f, 0.0f };
		cd.volumeGrowth = 1.5f;
		cd.height = 80.0f;
		cd.radius = 60.0f;

		controller = pxSys.controllerManager->createController(cd);

		debugCharacter.reset(new Egg::DebugPhysx{ device.Get(), psoManager.get(), controller->getActor() });

		debugGround.reset(new Egg::DebugPhysx{ device.Get(), psoManager.get(), pxSys.groundPlane });

		DirectX::XMMATRIX groundScaling = DirectX::XMMatrixScaling(2000.0f, 2000.0f, 2000.0f);
		
		DirectX::XMStoreFloat4x4A(&debugGround->offset, groundScaling);


		multi = Egg::Mesh::MultiMesh::Create();

		Egg::PipelineState::P pso = Egg::PipelineState::Create();
		pso->SetRootSignature(rootSig);
		pso->SetVertexShader(avatarVS);
		pso->SetPixelShader(avatarPS);
		pso->SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
		pso->SetDSVFormat(DXGI_FORMAT_D32_FLOAT);

		for(unsigned int i = 0; i < ybotModel.meshesLength; ++i) {

			Egg::Mesh::Geometry::P geom = Egg::Mesh::IndexedGeometry::Create(device.Get(), 
																			 ybotModel.meshes[i].vertices, ybotModel.meshes[i].verticesLength, ybotModel.meshes[i].vertexSize,
																			 ybotModel.meshes[i].indices, ybotModel.meshes[i].indicesLength * sizeof(unsigned int));
			geom->SetVertexType(ybotModel.meshes[i].vertexType);
			Egg::Material::P mat = Egg::Material::Create(psoManager.get(), geom, pso);

			mat->ConstantBufferSlot(0, PerFrameCb::id);
			mat->ConstantBufferSlot(1, PerObjectCb::id);
			mat->ConstantBufferSlot(2, BoneDataCb::id);

			multi->Add(geom, mat);
		}

		//da.reset(new DebugAnimation{ L"StrafeRight.fbx" });
		//da->CreateResources(device.Get(), psoManager.get());
	}

};
