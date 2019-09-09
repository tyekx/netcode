#pragma once

#include <Egg/SimpleApp.h>
#include <Egg/Importer.h>
#include <Egg/Math/Math.h>
#include <Egg/ConstantBuffer.hpp>
#include <Egg/Camera/Camera.h>
#include "ConstantBufferTypes.h"
#include <Egg/Scene.h>
#include <Egg/LuaScript.h>
#include <Egg/LuaManager.h>

using namespace Egg::Math;

class ggl005App : public Egg::SimpleApp {
public:
	Egg::Camera::BaseCamera cam;
	Egg::Mesh::Shaded::P shadedMesh;
	Float4x4 rotation;
	Egg::ConstantBuffer<PerFrameCb> pfcb;
	com_ptr<ID3D12DescriptorHeap> srvHeap;
	Egg::Texture2D tex;
	std::unique_ptr<LuaManager> luaManager;
	Egg::Scene scene;
	int selectionId;

	bool Keys[256];
public:

	virtual void Update(float dt, float T) override {
		if(Keys['W']) {
			cam.SetView(cam.Position + Float3::UnitZ * -dt, cam.Ahead);
		}
		if(Keys['S']) {
			cam.SetView(cam.Position + Float3::UnitZ * dt, cam.Ahead);
		}

		for(unsigned int i = 0; i < scene.GetObjectCount(); ++i) {
			Egg::GameObject * obj = scene[i];

			if(obj->HasComponent<ScriptComponent>()) {
				ScriptComponent * sc = obj->GetComponent<ScriptComponent>();
				sc->Script.InvokeUpdate(dt);
			}
		}

		pfcb->projMatrix = Float4x4::Identity; //cam.GetProjMatrix();
		pfcb->viewMatrix = Float4x4::Identity;//cam.GetViewMatrix();
		pfcb.Upload();
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

		ID3D12DescriptorHeap* descriptorHeaps[] = { srvHeap.Get() };
		commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		for(unsigned int i = 0; i < scene.GetObjectCount(); ++i) {
			Egg::GameObject * obj = scene[i];
			if(obj->HasComponent<MeshComponent>()) {
				MeshComponent *  mc = obj->GetComponent<MeshComponent>();

				if(obj->HasComponent<TransformComponent>()) {
					TransformComponent * tc = obj->GetComponent<TransformComponent>();
					Float4x4 mm = Float4x4::Translation(Float3{ 0,0,-0.5f })*
						Float4x4::Scaling(tc->Scale) * Float4x4::Rotation(tc->Rotation.xyz, tc->Rotation.w) * Float4x4::Translation(Float3{ 0,0, 0.5f }); //Float4x4::Translation(tc->Position.xyz / tc->Position.w);
					mc->Cbuffer->Model = mm;
					mc->Cbuffer->InvModel = mm.Invert();
				} else {
					mc->Cbuffer->Model = Float4x4::Identity;
					mc->Cbuffer->InvModel = Float4x4::Identity;
				}

				mc->Cbuffer.Upload();
				
				mc->ShadedMesh->SetPipelineState(commandList.Get());
				mc->ShadedMesh->BindConstantBuffer(commandList.Get(), pfcb);
				mc->ShadedMesh->BindConstantBuffer(commandList.Get(), mc->Cbuffer);
				// temporal fix for now
				commandList->SetGraphicsRootDescriptorTable(2, srvHeap->GetGPUDescriptorHandleForHeapStart());
				shadedMesh->Draw(commandList.Get());
			}

			
		}
		
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

		DX_API("Failed to close command list")
			commandList->Close();
	}

	/*
	Almost a render call
	*/
	void UploadResources() {
		DX_API("Failed to reset command allocator (UploadResources)")
			commandAllocator->Reset();
		DX_API("Failed to reset command list (UploadResources)")
			commandList->Reset(commandAllocator.Get(), nullptr);

		tex.UploadResource(commandList.Get());

		DX_API("Failed to close command list (UploadResources)")
			commandList->Close();

		ID3D12CommandList * commandLists[] = { commandList.Get() };
		commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		WaitForPreviousFrame();

		tex.ReleaseUploadResources();
	}
	
	virtual void Resize(int w, int h) override {
		Egg::SimpleApp::Resize(w, h);
		cam.SetAspect(aspectRatio);
	}

	virtual void CreateResources() override {


		Egg::SimpleApp::CreateResources();

		cam.SetView(Float3::Zero, Float3::UnitZ);
		cam.SetProj(3.14159235f / 4.0f, aspectRatio, 0.1f, 100.0f);

		ZeroMemory(Keys, sizeof(Keys));

		D3D12_DESCRIPTOR_HEAP_DESC dhd;
		dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		dhd.NodeMask = 0;
		dhd.NumDescriptors = 1;
		dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		DX_API("Failed to create descriptor heap for texture")
			device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(srvHeap.GetAddressOf()));
	}

	virtual void ReleaseResources() override {
		srvHeap.Reset();

		Egg::SimpleApp::ReleaseResources();
	}

	virtual void LoadAssets() override {
		selectionId = -1;
		pfcb.CreateResources(device.Get());

		luaManager = std::make_unique<LuaManager>();

		com_ptr<ID3DBlob> vertexShader = Egg::Shader::LoadCso(L"cbBasicVS.cso");
		com_ptr<ID3DBlob> pixelShader = Egg::Shader::LoadCso(L"DefaultPS.cso");
		com_ptr<ID3D12RootSignature> rootSig = Egg::Shader::LoadRootSignature(device.Get(), vertexShader.Get());

		CD3DX12_RASTERIZER_DESC rsd{ D3D12_DEFAULT };
		rsd.FillMode = D3D12_FILL_MODE_WIREFRAME;

		Egg::Mesh::Material::P material = Egg::Mesh::Material::Create();
		material->SetRootSignature(rootSig);
		material->SetVertexShader(vertexShader);
		material->SetPixelShader(pixelShader);
		material->SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
		material->SetDSVFormat(DXGI_FORMAT_D32_FLOAT);

		Egg::Mesh::Geometry::P geometry = Egg::Importer::ImportSimpleObj(device.Get(), L"giraffe.obj");

		shadedMesh = Egg::Mesh::Shaded::Create(psoManager.get(), material, geometry);
		/*
		auto go = scene.Next();
		go->SetName("Giraffe");
		go->AddComponent<TransformComponent>();
		MeshComponent * mc = go->AddComponent<MeshComponent>(device.Get());
		mc->ShadedMesh = shadedMesh;

		ScriptComponent * sc = go->AddComponent<ScriptComponent>(luaManager->GetScript(L"luatest.lua"), (void *)go);
		sc->Script.InvokeInit();
		*/

		tex = Egg::Importer::ImportTexture2D(device.Get(), L"giraffe.jpg");
		tex.CreateSRV(device.Get(), srvHeap.Get(), 0);
		UploadResources();
	}

};


