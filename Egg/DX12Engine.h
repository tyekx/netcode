#pragma once

#include "Common.h"
#include "Utility.h"
#include "Resource.h"

namespace Egg::Graphics {

	class IEngine {
	public:
		virtual ~IEngine() = default;
	};

}

namespace Egg::Graphics::Engine {

	enum class RenderItemState : unsigned {
		UNKNOWN = 0, IN_USE = 1, RETURNED = 2
	};

	class RenderItem {
	public:
		RenderItemState state;

		ID3D12RootSignature * rootSignature;
		ID3D12PipelineState * graphicsPso;

		int * cbAssoc;
		UINT cbAssocLength;

		D3D12_GPU_VIRTUAL_ADDRESS boneDataCbAddr;

		UINT texturesRootSigSlot;
		D3D12_GPU_DESCRIPTOR_HANDLE texturesHandle;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;

		D3D12_GPU_VIRTUAL_ADDRESS perMeshCbAddr;
		D3D_PRIMITIVE_TOPOLOGY primitiveTopology;

		UINT vertexCount;
		UINT indexCount;
	};

	struct RenderItemAllocation {
		UINT handleStart;
		
		RenderItem * items;
		UINT itemsLength;
	};

	class RenderItemAllocator {

		constexpr static int PAGE_SIZE = 512;

		struct RenderItemPage {
			UINT maxContigousBlock;
			UINT maxContigousBlockStartIndex;

			void * allocatedMemory;
			UINT allocatedMemoryLength;

			RenderItemPage * nextPage;
			RenderItemPage * prevPage;

			RenderItemPage() :
				maxContigousBlock{ static_cast<UINT>(PAGE_SIZE) }, 
				maxContigousBlockStartIndex{ 0U },
				allocatedMemory { std::malloc(PAGE_SIZE * sizeof(RenderItem)) },
				allocatedMemoryLength { static_cast<UINT>(PAGE_SIZE * sizeof(RenderItem)) },
				prevPage{ nullptr },
				nextPage{ nullptr } {

			}

			~RenderItemPage() {
				std::free(allocatedMemory);
				allocatedMemory = nullptr;
			}
		};

		RenderItemPage * startPage;
		RenderItemPage * endPage;

		void InsertFirstPage() {
			startPage = new (std::nothrow) RenderItemPage();

			if(startPage == nullptr) {
				__debugbreak();
			}

			endPage = startPage;
		}

		void InsertPage() {
			RenderItemPage * page = new (std::nothrow) RenderItemPage();

			if(page == nullptr) {
				__debugbreak();
			}

			page->nextPage = nullptr;
			page->prevPage = endPage;
			endPage->nextPage = page;
			endPage = page;
		}

	public:
		RenderItemAllocation Allocate(UINT numberOfItemsToAllocate) {

		}
	};

	class DX12Engine : public IEngine {
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
		void CreateResources(HWND parentWindow) {
			hwnd = parentWindow;

			CreateFactory();

			QueryAdapters();

			CreateDevice();

			CreateCommandQueue();

			CreateSwapChain();
		}

	};


}
