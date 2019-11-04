#pragma once

#include "Common.h"
#include "Utility.h"
#include "Resource.h"
#include "DX12RenderItem.h"
#include "DX12RenderItemAllocator.h"
#include "ResourceManager.h"
#include "IEngine.h"
#include <map>


namespace Egg::Graphics::DX12 {

	class Engine : public IEngine {
		HWND hwnd;

		com_ptr<IDXGIFactory5> dxgiFactory;

		com_ptr<IDXGISwapChain4> swapChain;
		UINT swapChainFlags;

		com_ptr<ID3D12Device5> device;
		com_ptr<ID3D12CommandQueue> commandQueue;
		
		com_ptr<IDXGIAdapter3> adapters[8];
		UINT adaptersLength;

		com_ptr<IDXGIOutput1> outputs[8];
		UINT outputsLength;

		ResourceManager resourceManager;

		DX12::RenderItemAllocator renderItemAllocator;
		
		std::map<UINT, RenderItemAllocation> allocationsCache;

		RenderItem ** renderItemBuffer;
		UINT renderItemBufferLength;
		

		void CreateFactory() {
			DX_API("Failed to create dxgi factory")
				CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
		}

		void CreateDevice() {
			com_ptr<ID3D12Device5> tempDevice;

			// always the 0th index will be tried first for creation, then we upgrade it as high as possible
			D3D_FEATURE_LEVEL featureLevels[] = {
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_12_0,
				D3D_FEATURE_LEVEL_12_1,
			};

			D3D12_FEATURE_DATA_FEATURE_LEVELS queryDataFeatureLevels;
			queryDataFeatureLevels.MaxSupportedFeatureLevel = featureLevels[0];
			queryDataFeatureLevels.NumFeatureLevels = _countof(featureLevels);
			queryDataFeatureLevels.pFeatureLevelsRequested = featureLevels;

			DX_API("Failed to create device with %s", Egg::Utility::FeatureLevelToString(queryDataFeatureLevels.MaxSupportedFeatureLevel))
				D3D12CreateDevice(nullptr, queryDataFeatureLevels.MaxSupportedFeatureLevel, IID_PPV_ARGS(tempDevice.GetAddressOf()));

			DX_API("Failed to query supported feature levels")
				tempDevice->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &queryDataFeatureLevels, sizeof(D3D12_FEATURE_DATA_FEATURE_LEVELS));

			tempDevice.Reset();

			DX_API("Failed to upgrade device to %s", Egg::Utility::FeatureLevelToString(queryDataFeatureLevels.MaxSupportedFeatureLevel))
				D3D12CreateDevice(nullptr, queryDataFeatureLevels.MaxSupportedFeatureLevel, IID_PPV_ARGS(tempDevice.GetAddressOf()));

			Egg::Utility::Debugf("Created DX12 device with %s", Egg::Utility::FeatureLevelToString(queryDataFeatureLevels.MaxSupportedFeatureLevel));

			device = std::move(tempDevice);
		}

		void ClearAdapters() {
			for(UINT i = 0; i < adaptersLength; ++i) {
				adapters[i].Reset();
			}
			adaptersLength = 0;
		}

		void QueryAdapters() {
			if(adaptersLength != 0) {
				ClearAdapters();
			}

			com_ptr<IDXGIAdapter1> tempAdapter;

			for(UINT i = 0; dxgiFactory->EnumAdapters1(i, tempAdapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i) {
				DX_API("Failed to cast IDXGIAdapter1 to IDXGIAdapter3")
					tempAdapter.As(&adapters[adaptersLength]);

				DXGI_ADAPTER_DESC2 adapterDesc;
				
				DX_API("Failed to query adapter desc")
					adapters[adaptersLength]->GetDesc2(&adapterDesc);

				Egg::Utility::Debugf("Graphics adapter: %S", adapterDesc.Description);

				++adaptersLength;
			}
		}

		void CreateCommandQueue() {
			D3D12_COMMAND_QUEUE_DESC cqd;
			cqd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			cqd.NodeMask = 0;
			cqd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			cqd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

			DX_API("Failed to create direct command queue")
				device->CreateCommandQueue(&cqd, IID_PPV_ARGS(commandQueue.GetAddressOf()));
		}

		void QuerySyncSupport() {
			BOOL allowTearing;

			DX_API("Failed to query dxgi feature support: allow tearing")
				dxgiFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));

			swapChainFlags = (allowTearing) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		}

		void CreateSwapChain() {
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
			// if you specify width/height as 0, the CreateSwapChainForHwnd will query it from the output window
			swapChainDesc.Width = 0;
			swapChainDesc.Height = 0;
			swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.Stereo = false;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = 2;
			swapChainDesc.Scaling = DXGI_SCALING_NONE;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			swapChainDesc.Flags = swapChainFlags | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

			com_ptr<IDXGISwapChain1> tempSwapChain;

			DX_API("Failed to create swap chain for hwnd")
				dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, tempSwapChain.GetAddressOf());

			DX_API("Failed to cast swap chain")
				tempSwapChain.As(&swapChain);
			
			DX_API("Failed to make window association")
				dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
		}

		

	public:
		virtual void CreateResources(void* parentWindow) override {
			hwnd = static_cast<HWND>(parentWindow);

			CreateFactory();

			QueryAdapters();

			CreateDevice();

			CreateCommandQueue();

			CreateSwapChain();
		}

		void BeginRender() {
			renderItemBufferLength = 0;
		}

		void Render(UINT item) {
			ASSERT(allocationsCache.find(item) != allocationsCache.end(), "allocation was not found");

			RenderItemAllocation & alloc = allocationsCache[item];
			for(UINT i = 0; i < alloc.itemsLength; ++i) {
				renderItemBuffer[renderItemBufferLength++] = alloc.items + i;
			}
		}

		void Render(RenderItem * item) {

		}

		void EndRender() {
			for(UINT i = 0; i < renderItemBufferLength; ++i) {
				Render(renderItemBuffer[i]);
			}
		}

		virtual UINT GetHandle(UINT numberOfItems) override {
			RenderItemAllocation alloc = renderItemAllocator.Allocate(numberOfItems);

			ASSERT(allocationsCache.find(alloc.handleStart) == allocationsCache.end(), "Overwriting renderitem allocation");

			allocationsCache[alloc.handleStart] = alloc;
			
			return alloc.handleStart;
		}

		virtual Model LoadAssets(UINT handle, Asset::Model * model) override {
			ASSERT(allocationsCache.find(handle) != allocationsCache.end(), "Failed to find allocation");

			RenderItemAllocation & alloc = allocationsCache[handle];

			Model m;

			resourceManager.Compose(alloc.items, model, m);

			return m;
		}
	};


}
