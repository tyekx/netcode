#pragma once

#include "Utility.h"
#include "Modules.h"
#include <map>

#include "DX12Common.h"
#include "DX12ResourceUploadBatch.h"
#include "DX12Resource.h"
#include "DX12RenderItem.h"
#include "DX12RenderItemAllocator.h"
#include "DX12ShaderManager.h"
#include "DX12TextureLibrary.h"
#include "DX12PipelineStateManager.h"
#include "DX12GeometryManager.h"
#include "DX12MaterialManager.h"
#include "DX12RenderItemCollection.h"
#include "DX12CbufferAllocator.h"
#include "DX12SpriteFontLibrary.h"

namespace Egg::Graphics::DX12 {

	class FrameResource {
	public:
		com_ptr<ID3D12Resource> swapChainBuffer;
		com_ptr<ID3D12GraphicsCommandList2> commandList;
		com_ptr<ID3D12CommandAllocator> commandAllocator;
		com_ptr<ID3D12Fence1> fence;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
		UINT64 fenceValue;
		HANDLE fenceEvent;
		D3D12_RECT scissorRect;
		D3D12_VIEWPORT viewPort;
		D3D12_CLEAR_VALUE dsvClearValue;
		D3D12_CLEAR_VALUE rtvClearValue;
		std::unique_ptr<Resource::IResourceUploadBatch> resourceUploader;

		ID3D12GraphicsCommandList2 * GetCommandList() const {
			return commandList.Get();
		}

		void CreateResources(ID3D12Device * device) {
			DX_API("Failed to create command allocator")
				device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator.GetAddressOf()));

			DX_API("Failed to create direct command list")
				device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf()));

			DX_API("Failed to initially close command list")
				commandList->Close();

			fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			if(fenceEvent == NULL) {
				DX_API("Failed to create windows event") HRESULT_FROM_WIN32(GetLastError());
			}
			fenceValue = 0;

			DX_API("Failed to create fence")
				device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));

			resourceUploader = std::make_unique<Resource::ResourceUploadBatch>();
			resourceUploader->CreateResources(device);
		}
		
		void WaitForCompletion() {
			const UINT64 cv = fenceValue;

			if(fence->GetCompletedValue() < cv) {
				DX_API("Failed to set winapi event")
					fence->SetEventOnCompletion(cv, fenceEvent);
				WaitForSingleObject(fenceEvent, INFINITE);
			}

			++fenceValue;
		}

		void FinishRecording() {
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(swapChainBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

			DX_API("Failed to close command list")
				commandList->Close();
		}

		void ReleaseResources() {
			commandList.Reset();
			commandAllocator.Reset();
			fence.Reset();
			fenceValue = 0;
		}
	};

	class DX12GraphicsModule : public Egg::Module::IGraphicsModule {
		HWND hwnd;
		com_ptr<ID3D12Debug3> debugController;

		com_ptr<IDXGIFactory5> dxgiFactory;

		com_ptr<IDXGISwapChain4> swapChain;
		UINT swapChainFlags;

		com_ptr<ID3D12Device5> device;
		com_ptr<ID3D12CommandQueue> commandQueue;
		com_ptr<ID3D12CommandQueue> copyCommandQueue;
		
		com_ptr<IDXGIAdapter3> adapters[8];
		UINT adaptersLength;

		com_ptr<IDXGIOutput1> outputs[8];
		UINT outputsLength;

		com_ptr<ID3D12DescriptorHeap> rtvDescHeap;
		UINT rtvDescHeapIncrement;
		UINT backbufferDepth;
		UINT width;
		UINT height;

		com_ptr<ID3D12DescriptorHeap> dsvDescHeap;
		com_ptr<ID3D12Resource> dsvResource;
		DXGI_FORMAT depthStencilFormat;
		D3D12_CLEAR_VALUE dsvClearValue;
		D3D12_CLEAR_VALUE rtvClearValue;

		std::vector<FrameResource> frameResources;
		UINT backbufferIndex;
		UINT presentedBackbufferIndex;

		std::unique_ptr<SpriteBatch> spriteBatch;
		std::vector<DX12::RenderItem*> renderItemBuffer;
		float aspectRatio;

		void NextBackBufferIndex();

		void CreateFactory();

		void CreateDevice();

		void ClearAdapters();

		void QueryAdapters();

		void CreateCommandQueue();

		void QuerySyncSupport();

		void CreateSwapChain();

		ShaderManager shaderManager;
		TextureLibrary textureLibrary;
		PipelineStateManager psManager;
		GeometryManager geomManager;
		MaterialManager matManager;
		RenderItemCollection renderItemColl;
		CbufferAllocator cbufferAllocator;
		SpriteFontLibrary fontLibrary;

	public:

		virtual void Prepare() override;

		virtual void SetRenderTarget() override;

		virtual void SetRenderTarget(HRENDERTARGET rt) override;

		virtual void ClearRenderTarget() override;

		virtual void Record(HITEM item) override;

		virtual void Render() override;

		virtual void Present() override;

		virtual void Start(Module::AApp * app) override;

		virtual void Shutdown() override;

		virtual void OnResized(int width, int height) override;

		virtual float GetAspectRatio() const override;

		virtual HITEM CreateItem() override {
			return renderItemColl.CreateItem();
		}

		virtual HSHADER LoadShader(const std::wstring & shaderPath) override {
			return shaderManager.LoadShader(shaderPath);
		}

		virtual HINCOMPLETESHADER CreateVertexShader() override {
			return shaderManager.CreateVertexShader();
		}

		virtual HINCOMPLETESHADER CreatePixelShader() override {
			return shaderManager.CreatePixelShader();
		}

		virtual HINCOMPLETESHADER CreateGeometryShader() override {
			return shaderManager.CreateGeometryShader();
		}

		virtual HINCOMPLETESHADER CreateDomainShader() override {
			return shaderManager.CreateDomainShader();
		}

		virtual HINCOMPLETESHADER CreateHullShader() override {
			return shaderManager.CreateHullShader();
		}

		virtual void SetShaderEntry(HINCOMPLETESHADER shader, const std::string & entryFunction) override {
			shaderManager.SetShaderEntry(shader, entryFunction);
		}

		virtual void SetShaderSource(HINCOMPLETESHADER shader, const std::wstring & shaderPath) override {
			shaderManager.SetShaderSource(shader, shaderPath);
		}

		virtual void SetShaderMacros(HINCOMPLETESHADER shader, const std::map<std::string, std::string> & defines) override {
			shaderManager.SetShaderMacros(shader, defines);
		}

		virtual HSHADER CompileShader(HINCOMPLETESHADER shader) override {
			return shaderManager.CompileShader(shader);
		}

		virtual HTEXTURE LoadTexture(const std::wstring & textureMediaPath) override {
			return textureLibrary.LoadTexture2D(textureMediaPath);
		}

		virtual HPSO CreatePipelineState() override {
			return psManager.Create();
		}

		virtual void SetVertexShader(HPSO pso, HSHADER vertexShader) override {
			psManager.SetVertexShader(pso, shaderManager.GetShader(vertexShader));
		}

		virtual void SetPixelShader(HPSO pso, HSHADER pixelShader) override {
			psManager.SetPixelShader(pso, shaderManager.GetShader(pixelShader));
		}

		virtual void SetGeometryShader(HPSO pso, HSHADER geometryShader) override {
			psManager.SetGeometryShader(pso, shaderManager.GetShader(geometryShader));
		}

		virtual void SetHullShader(HPSO pso, HSHADER hullShader) override {
			psManager.SetHullShader(pso, shaderManager.GetShader(hullShader));
		}

		virtual void SetDomainShader(HPSO pso, HSHADER domainShader) override {
			psManager.SetDomainShader(pso, shaderManager.GetShader(domainShader));
		}

		virtual HGEOMETRY CreateGeometry(EGeometryType type = EGeometryType::INDEXED) override {
			return geomManager.CreateGeometry(type);
		}

		virtual void AddVertexBufferLOD(HGEOMETRY geometry, void * ptr, unsigned int sizeInBytes, unsigned int strideInBytes) override {
			geomManager.AddVertexBufferLOD(geometry, ptr, sizeInBytes, strideInBytes);
		}

		virtual void AddIndexBufferLOD(HGEOMETRY geometry, void * ptr, unsigned int sizeInBytes, unsigned int  format) override {
			geomManager.AddIndexBufferLOD(geometry, ptr, sizeInBytes, static_cast<DXGI_FORMAT>(format));
		}

		virtual void AddInputElement(HGEOMETRY geometry, const char * name, unsigned int semanticIndex, unsigned int  format, unsigned int byteOffset) override {
			geomManager.AddInputElement(geometry, name, semanticIndex, static_cast<DXGI_FORMAT>(format), byteOffset);
		}

		virtual void AddInputElement(HGEOMETRY geometry, const char * name, unsigned int  format, unsigned int byteOffset) override {
			AddInputElement(geometry, name, 0, format, byteOffset);
		}

		virtual HMATERIAL CreateMaterial(HPSO pso, HGEOMETRY geometry) override {
			return matManager.CreateMaterial(psManager.GetGPSO(pso), geomManager.GetInputLayout(geometry));
		}

		virtual void SetMaterial(HITEM item, HMATERIAL material) override {
			matManager.SetRenderItemMaterial(renderItemColl.GetItem(item), material);
		}

		virtual void SetGeometry(HITEM item, HGEOMETRY geometry) override {
			geomManager.SetRenderItemGeometry(renderItemColl.GetItem(item), geometry);
		}


		virtual void AllocateTextures(HITEM item, unsigned int  numTextures) override {
			textureLibrary.AllocateTextures(renderItemColl.GetItem(item), numTextures);
		}

		virtual void SetTexture(HITEM item, unsigned int  slot, HTEXTURE texture) override {
			textureLibrary.SetTexture(renderItemColl.GetItem(item), slot, texture);
		}

		virtual HTEXTURE SetTexture(HITEM item, unsigned int  slot, const std::wstring & texturePath) override {
			HTEXTURE t = textureLibrary.LoadTexture2D(texturePath);
			SetTexture(item, slot, t);
			return t;
		}

		virtual HCBUFFER AllocateCbuffer(unsigned int sizeInBytes) override {
			return cbufferAllocator.AllocateCbuffer(sizeInBytes);
		}

		virtual void * GetCbufferPointer(HCBUFFER cbuffer) override {
			return cbufferAllocator.GetCbufferPointer(cbuffer);
		}
		
		virtual void AddCbuffer(HITEM item, HCBUFFER cbuffer, unsigned int slot) override {
			cbufferAllocator.AddRenderItemCbuffer(renderItemColl.GetItem(item), cbuffer, slot);
		}

		virtual void SetCbuffer(HITEM item, HCBUFFER cbuffer, unsigned int idx, unsigned int slot) override {
			cbufferAllocator.SetRenderItemCbuffer(renderItemColl.GetItem(item), cbuffer, idx, slot);
		}

		virtual UINT GetCbufferSlot(HITEM item, const std::string & cbufferName) override {
			return matManager.GetCbufferSlot(renderItemColl.GetItem(item), cbufferName);
		}

		void ReleaseSwapChainResources();

		void CreateSwapChainResources();

		virtual HFONT LoadFont(const std::wstring & fontName) override;
		virtual void TestFont(HFONT font) override;
	};


}
