#pragma once

#include <Egg/SimpleApp.h>
#include <Egg/Importer.h>
#include <Egg/Math/Math.h>
#include <Egg/ConstantBuffer.hpp>
#include <Egg/Camera/Camera.h>
#include <Egg/Input.h>
#include <Egg/PhysxSystem.h>
#include <Egg/Scene.h>
#include <Egg/Mesh/MultiMesh.h>

using namespace Egg::Math;

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
		animT += dt;


		Egg::Asset::Animation & a = ybotModel.animations[2];
		animT = fmodf(animT, (float)a.duration);
		
		float timeSinceLastTick = 0.0f;
		float timeGap = 0.0f;
		unsigned int keysId = 0;
		for(unsigned int i = 1; i < a.boneData[0].keysLength; ++i) {
			if(animT >= (float)a.boneData[0].keys[i - 1].time && (float)a.boneData[0].keys[i].time >= animT) {
				keysId = i;
				timeSinceLastTick = animT - a.boneData[0].keys[i - 1].time;
				timeGap = a.boneData[0].keys[i].time - a.boneData[0].keys[i - 1].time;
				break;
			}
		}

		float lerpArg = timeSinceLastTick / timeGap;

		for(unsigned int i = 0; i < a.boneDataLength; ++i) {
			Float3 posA = a.boneData[i].keys[keysId - 1].position;
			Float3 posB = a.boneData[i].keys[keysId].position;
			Float4x4 T = Float4x4::Translation(posA * (1 - lerpArg) + posB * lerpArg);

			Float4 quatA = a.boneData[i].keys[keysId - 1].rotation;
			Float4 quatB = a.boneData[i].keys[keysId].rotation;
			Float4x4 R = Float4x4::Rotation(quatA * (1 - lerpArg) + quatB * lerpArg);

			Float3 scaleA = a.boneData[i].keys[keysId - 1].scale;
			Float3 scaleB = a.boneData[i].keys[keysId].scale;
			Float4x4 S = Float4x4::Scaling(scaleA * (1 - lerpArg) + scaleB * lerpArg);

			boneDataCb->BindTransform[i] = S * R * T;
		}

		Float4x4 toRoot[64];
		toRoot[0] = boneDataCb->BindTransform[0];
		for(unsigned int i = 1; i < a.boneDataLength; ++i) {
			toRoot[i] = boneDataCb->BindTransform[i] * toRoot[ybotModel.bones[i].parentId];
		}

		for(unsigned int i = 0; i < a.boneDataLength; ++i) {
			boneDataCb->BindTransform[i] = ybotModel.bones[i].transform * toRoot[i];
		}

		boneDataCb.Upload();

	}

	virtual void Update(float dt, float T) override {
		pxSys.Simulate(dt);

		float scale = 0.001f;
		cb->Model = Float4x4::Scaling(Float3{ scale, scale, scale }) * Float4x4::Translation(Float3{ 0.0f, 0.0f, 0.3f });
		cb->InvModel = cb->Model.Invert();
		cb.Upload();

		UpdateAnimation(dt);

		Float3 movementVector = Float3::Zero;
		movementVector += Float3::UnitZ * Egg::Input::GetAxis("Vertical");
		movementVector += Float3::UnitX * Egg::Input::GetAxis("Horizontal");
		if(movementVector.LengthSquared() > 0.0f) {
			movementVector = movementVector.Normalize();
			baseCam.Position += movementVector * speed * dt;
		}

		Int2 mouseDelta = Egg::Input::GetMouseDelta();
		Float2 screenSize{ viewPort.Width, viewPort.Height };
		Float2 mouseDeltaf{ (float)mouseDelta.x, (float)mouseDelta.y };
		Float2 normalizedDelta = mouseDeltaf / screenSize;

		Float4x4 aheadRotation = Float4x4::Rotation(Float3::UnitY, normalizedDelta.x * mouseSpeed) * Float4x4::Rotation(Float3::UnitX, normalizedDelta.y * mouseSpeed);

		baseCam.Ahead = aheadRotation.Transform(Float4{ baseCam.Ahead, 1.0f }).xyz;

		baseCam.UpdateMatrices();
		perFrameCb->View = baseCam.GetViewMatrix();
		perFrameCb->Proj = baseCam.GetProjMatrix();
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
		Egg::Input::MouseMove(Int2{ x, y });
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

		/*
		* Run render system here
		*/
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
		baseCam.SetProj(0.9f, aspectRatio, 0.0001f, 100.0f);
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
		mouseSpeed = 3.5f;

		Egg::Input::SetAxis("Vertical", 'W', 'S');
		Egg::Input::SetAxis("Horizontal", 'D', 'A');

		baseCam.Ahead = Float3::UnitZ;
		baseCam.Position = Float3::Zero;
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
	}

};
