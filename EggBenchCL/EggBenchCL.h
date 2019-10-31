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
#include <Egg/Shader.h>

#include <Egg/Utility.h>
#include <Egg/BasicGeometry.h>
#include "Entity.h"
#include <thread>
#include <condition_variable>

#include <mutex>
#include <condition_variable>
#include <ppl.h>
#include <atomic>

class Semaphore {
public:
	Semaphore(int count_ = 0)
		: count(count_)
	{
	}

	inline void NotifyAll(int n) {
		std::unique_lock<std::mutex> lock(mtx);
		count += n;
		cv.notify_all();
	}

	inline void Wait(int n = 1) {
		std::unique_lock<std::mutex> lock(mtx);
		while(count < n) {
			cv.wait(lock);
		}
		count -= n;
	}
private:
	std::mutex mtx;
	std::condition_variable cv;
	int count;
};

class EggBenchCL : public Egg::SimpleApp {
protected:
	Egg::ConstantBuffer<PerFrameCb> perFrameCb;
	Egg::Camera::BaseCamera baseCam;
	std::unique_ptr<Egg::PsoManager> psoManager;
	std::vector<Entity> entities;
	int objCount;
	com_ptr<ID3D12RootSignature> rootSig;
	Egg::Stopwatch sw;
	float simTime;
	float popTime;
	float drawTime;
	ReadOptimizedCbVector<PerObjectCb> perObjCbs;
	Egg::Material::P material;
	Egg::Mesh::Shaded::P shaded;
	std::atomic<int> updateJobId;
	std::atomic<int> renderJobId;
	std::atomic<bool> exitFlag;
	Semaphore renderSema;
	Semaphore urSema;
	Semaphore updateSema;
	int numThreads;
	std::vector<std::thread> workerThreads;
	float deltaTime;
	Egg::Texture2D diffuseTex;
	com_ptr<ID3D12DescriptorHeap> texDh;
public:

	EggBenchCL() : Egg::SimpleApp{}, perFrameCb{}, baseCam{}, psoManager{ nullptr }, entities{}, objCount{ 2048 }, sw{} {
		sw.Reset();
	}

	virtual void Render() override {
		PopulateCommandList();

		sw.Start();

		// Execute
		ID3D12CommandList * cLists[] = { commandList.Get() };
		commandQueue->ExecuteCommandLists(_countof(cLists), cLists);

		DX_API("Failed to present swap chain")
			swapChain->Present(1, 0);

		// Sync
		WaitForPreviousFrame();

		sw.Stop();

		drawTime = sw.GetElapsedSeconds();
		Egg::Utility::Debugf("SIM: %1.12f | POP: %1.12f | GPU: %1.12f\r\n", simTime, popTime, drawTime);
	}

	virtual void Update(float dt, float T) override {
		sw.Start();

		deltaTime = dt;
		updateJobId.store(0);
		updateSema.NotifyAll(numThreads);
		urSema.Wait(numThreads);

		sw.Stop();
		simTime = sw.GetElapsedSeconds();

		Egg::Input::Reset();
	}

	virtual void KeyPressed(uint32_t keyCode) override {
		Egg::Input::KeyPressed(keyCode);
	}

	virtual void KeyReleased(uint32_t keyCode) override {
		Egg::Input::KeyReleased(keyCode);
	}

	virtual void Blur() override {
		//Egg::Input::Blur();
	}

	virtual void Focused() override {
		//Egg::Input::Focused();
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

		commandList->SetDescriptorHeaps(1, texDh.GetAddressOf());
		commandList->SetGraphicsRootSignature(rootSig.Get());
		commandList->SetPipelineState(material->GetPSO());
		commandList->SetGraphicsRootConstantBufferView(1, perFrameCb.GetGPUVirtualAddress());

		UINT incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		sw.Start();

		perObjCbs.CopyOnGpu(commandList.Get());


		CD3DX12_GPU_DESCRIPTOR_HANDLE handle{ texDh->GetGPUDescriptorHandleForHeapStart() };
		ID3D12GraphicsCommandList * gcl = commandList.Get();

		for(int idx = 0; idx < objCount; ++idx) {
			gcl->SetGraphicsRootDescriptorTable(2, handle);
			entities[idx].Draw(gcl);
			handle.Offset(incrementSize);
		}

		sw.Stop();
		popTime = sw.GetElapsedSeconds();



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

		diffuseTex.UploadResource(commandList.Get());

		DX_API("Failed to close command list (UploadResources)")
			commandList->Close();

		ID3D12CommandList * commandLists[] = { commandList.Get() };
		commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		WaitForPreviousFrame();

		diffuseTex.ReleaseUploadResources();
	}

	virtual void CreateResources() override {
		Egg::SimpleApp::CreateResources();
		psoManager = std::make_unique<Egg::PsoManager>(device.Get());
		baseCam.NearPlane = 1.0f;
		baseCam.FarPlane = 10000.0f;
		baseCam.Ahead = DirectX::XMFLOAT3{ 0.0f, 0.0f, -1.0f };
		baseCam.Position = DirectX::XMFLOAT3{ 0.0f, 0.0f, 1500.0f };
		baseCam.UpdateMatrices();


		numThreads = 12;

		ID3D12GraphicsCommandList * cl;

		workerThreads.reserve(2 * numThreads);

		for(int i = 0; i < numThreads; ++i) {
			workerThreads.emplace_back([this]() {
				while(true) {
					updateSema.Wait(1);

					if(exitFlag) {
						return;
					}

					int jobId;
					while((jobId = updateJobId++) < objCount) {
						entities[jobId].Animate(deltaTime);
					}
					urSema.NotifyAll(1);
				}
			});/*
			workerThreads.emplace_back([this, cl]() {
				while(true) {
					renderSema.Wait(1);

					if(exitFlag) {
						return;
					}

					int jobId;
					while((jobId = renderJobId++) < objCount) {
						entities[jobId].Animate(deltaTime);
					}
				}
			});*/
		}
	}

	virtual void ReleaseResources() override {
		Egg::SimpleApp::ReleaseResources();

		exitFlag.store(true);
		updateSema.NotifyAll(numThreads);
		renderSema.NotifyAll(numThreads);

		for(int i = 0; i < (numThreads); ++i) {
			workerThreads[i].join();
		}
	}

	virtual void LoadAssets() override {

		perFrameCb.CreateResources(device.Get());

		com_ptr<ID3DBlob> defVS = Egg::ShaderProgram::LoadCso(L"DefaultVS.cso");
		com_ptr<ID3DBlob> defPS = Egg::ShaderProgram::LoadCso(L"DefaultPS.cso");
		rootSig = Egg::ShaderProgram::LoadRootSignature(device.Get(), defVS.Get());

		Egg::PipelineState::P pso = Egg::PipelineState::Create();
		pso->SetRootSignature(rootSig);
		pso->SetVertexShader(defVS);
		pso->SetPixelShader(defPS);
		pso->SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
		pso->SetDSVFormat(DXGI_FORMAT_D32_FLOAT);

		Egg::Mesh::Geometry::P cubeGeom = Egg::BasicGeometry::CreateBox(device.Get());

		material = Egg::Material::Create(psoManager.get(), cubeGeom, pso);

		shaded = Egg::Mesh::Shaded::Create(cubeGeom, material, nullptr);

		diffuseTex = Egg::Importer::ImportTexture2D(device.Get(), L"giraffe.jpg");

		
		D3D12_DESCRIPTOR_HEAP_DESC dhd;
		dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		dhd.NodeMask = 0;
		dhd.NumDescriptors = objCount;
		dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		DX_API("Failed to create descriptor heap")
			device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(texDh.GetAddressOf()));

		DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4A(&baseCam.GetViewMatrix());
		DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4A(&baseCam.GetProjMatrix());
		DirectX::XMMATRIX vp = DirectX::XMMatrixMultiply(view, proj);

		DirectX::XMStoreFloat4x4A(&perFrameCb->ViewProj, DirectX::XMMatrixTranspose(vp));
		perFrameCb->eyePos = DirectX::XMFLOAT3A{ baseCam.Position.x, baseCam.Position.y, baseCam.Position.z };
		perFrameCb->Light.position = DirectX::XMFLOAT4A{ 1.0f, 0.0f, 0.0f, 0.0f };
		perFrameCb->Light.intensity = DirectX::XMFLOAT3A{ 1.0f, 1.0f, 1.0f };
		perFrameCb.Upload();

		entities.reserve(objCount);

		perObjCbs.CreateResources(device.Get());

		for(int i = 0; i < objCount; ++i) {
			Entity& e = entities.emplace_back();
			float x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float y = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float z = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float w = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			x = 2.0f * x - 1.0f;
			y = 2.0f * y - 1.0f;
			z = 2.0f * z - 1.0f;
			e.animTime = 4.0f * w;
			e.direction = DirectX::XMFLOAT3(x, y, z);
			e.shaded.material = material.get();
			e.shaded.geometry = cubeGeom.get();
			e.scale = DirectX::XMFLOAT3{ 1,1,1 };
			e.objCb = perObjCbs.Next();
			e.CreateResources(device.Get());
			diffuseTex.CreateSRV(device.Get(), texDh.Get(), i);
			//diffuseTex.CreateSRV(device.Get(), e.texHeap.Get(), 0);
		}

		UploadResources();
	}

};
