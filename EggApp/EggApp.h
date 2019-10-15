#pragma once

#include <Egg/SimpleApp.h>
#include <Egg/Importer.h>
#include <Egg/ConstantBuffer.hpp>
#include <Egg/Camera/Camera.h>
#include <Egg/PhysxSystem.h>
#include <Egg/Scene.h>
#include <Egg/Mesh/MultiMesh.h>
#include <Egg/Input.h>

class EggApp : public Egg::SimpleApp {
protected:
	Egg::Mesh::MultiMesh::P multi;
	Egg::Asset::Model ybotModel;
	Egg::ConstantBuffer<PerObjectCb> cb;
	Egg::ConstantBuffer<PerFrameCb> perFrameCb;
	Egg::ConstantBuffer<BoneDataCb> boneDataCb;
	Egg::Camera::BaseCamera baseCam;
	Egg::PhysxSystem pxSys;
	Egg::Scene scene;
	float speed;
	float mouseSpeed;
	float animT;
public:

	EggApp() : Egg::SimpleApp{}, multi{}, ybotModel{}, cb{}, perFrameCb{}, boneDataCb{}, baseCam{}, pxSys{}, scene{}, speed{}, mouseSpeed{}, animT{ 0.0f } { }
	
	void UpdateAnimation(float dt) {
		animT += dt * 60.0f;

		Egg::Asset::Animation & a = ybotModel.animations[0];
		animT = fmodf(animT, (float)a.duration);
		
		float timeSinceLastTick = 0.0f;
		float timeGap = 0.0f;
		unsigned int keysId = 0;
		for(unsigned int i = 1; i < a.keysLength; ++i) {
			if(animT >= (float)(a.times[i - 1]) && (float)(a.times[i]) >= animT) {
				keysId = i;
				timeSinceLastTick = animT - (float)(a.times[i - 1]);
				timeGap = (float)((a.times[i]) - (a.times[i - 1]));
				break;
			}
		}

		float lerpArg = timeSinceLastTick / timeGap;

		DirectX::XMFLOAT4 unitW{ 0.0f, 0.0f, 0.0f, 1.0f };
		DirectX::XMVECTOR rotationSource = DirectX::XMLoadFloat4(&unitW);
		DirectX::XMFLOAT4X4A locals[64];

		Egg::Asset::AnimationKey * keysPrev = a.keys + ((keysId -1)* a.bonesLength);
		Egg::Asset::AnimationKey * keysNext = a.keys + (keysId * a.bonesLength);
		for(unsigned int i = 0; i < a.bonesLength; ++i) {

			DirectX::XMVECTOR posA = DirectX::XMLoadFloat3(&keysPrev[i].position);
			DirectX::XMVECTOR posB = DirectX::XMLoadFloat3(&keysNext[i].position);
			DirectX::XMVECTOR T = DirectX::XMVectorLerp(posA, posB, lerpArg);

			DirectX::XMVECTOR quatA = DirectX::XMLoadFloat4(&keysPrev[i].rotation);
			DirectX::XMVECTOR quatB = DirectX::XMLoadFloat4(&keysNext[i].rotation);
			DirectX::XMVECTOR R = DirectX::XMQuaternionSlerp(quatA, quatB, lerpArg);

			DirectX::XMVECTOR scaleA = DirectX::XMLoadFloat3(&keysPrev[i].scale);
			DirectX::XMVECTOR scaleB = DirectX::XMLoadFloat3(&keysNext[i].scale);
			DirectX::XMVECTOR S = DirectX::XMVectorLerp(scaleA, scaleB, lerpArg);

			DirectX::XMMATRIX srt = DirectX::XMMatrixAffineTransformation(S, rotationSource, R, T);
			DirectX::XMStoreFloat4x4A(&(locals[i]), srt);
		}


		DirectX::XMFLOAT4X4A toRoot[64];
		toRoot[0] = locals[0];
		for(unsigned int i = 1; i < a.bonesLength; ++i) {
			DirectX::XMMATRIX local = DirectX::XMLoadFloat4x4A(&(locals[i]));
			DirectX::XMMATRIX root = DirectX::XMLoadFloat4x4A(&(toRoot[ybotModel.bones[i].parentId]));
			DirectX::XMStoreFloat4x4A(&(toRoot[i]), DirectX::XMMatrixMultiply(local, root));
		}

		for(unsigned int i = 0; i < a.bonesLength; ++i) {
			DirectX::XMMATRIX root = DirectX::XMLoadFloat4x4A(&(toRoot[i]));
			DirectX::XMMATRIX offset = DirectX::XMLoadFloat4x4(&ybotModel.bones[i].transform);
			DirectX::XMStoreFloat4x4A(&(boneDataCb->BindTransform[i]), DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(offset, root)));
		}

		boneDataCb.Upload();

	}

	virtual void Update(float dt, float T) override {
		pxSys.Simulate(dt);
		float scale = 0.01f;

		DirectX::XMStoreFloat4x4A(&cb->Model, DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4A(&cb->InvModel, DirectX::XMMatrixIdentity());
		cb.Upload();

		

		UpdateAnimation(dt);


		baseCam.UpdateMatrices();

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

		pxSys.CreateResources();

		speed = 1.0f;
		mouseSpeed = 1.5f;
		baseCam.NearPlane = 1.0f;
		baseCam.FarPlane = 1000.0f;
		baseCam.Ahead = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };
		baseCam.Position = DirectX::XMFLOAT3{ 0.0f, 75.0f, 180.0f };
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
