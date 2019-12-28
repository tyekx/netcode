#pragma once

#include "DX12Common.h"
#include "RenderPass.h"

#include <DirectXTex/DirectXTex.h>
#include <set>

/*
ezeket mind lehet egy handle mögé tenni

deallokáláshoz kell:
- milyen típusú adatról van szó, perzisztencia + heap típusból megvan
- ID3D12Resource * pointer
rajzoláshoz kell:
- buffer location,
- sizeInBytes
- strideInBytes
- DXGI_FORMAT

- mindenféle nézetek, UAV, SRV, CBV, RENDER_TARGET, DEPTH_STENCIL
*/

namespace Egg::Graphics::DX12 {


	struct ResourceDesc {
		ID3D12Resource * resource;
		D3D12_GPU_VIRTUAL_ADDRESS bufferLocation;
		UINT64 sizeInBytes;
		UINT64 width;
		UINT height;
		UINT strideInBytes;
		UINT16 mipLevels;
		UINT16 depth;
		ResourceType type;
		DXGI_FORMAT format;
		D3D12_RESOURCE_FLAGS flags;
		D3D12_RESOURCE_STATES state;
		D3D12_RESOURCE_DIMENSION dimension;
		D3D12_CLEAR_VALUE clearValue;

		D3D12_HEAP_TYPE GetHeapType() const noexcept {
			switch(type) {
			case ResourceType::PERMANENT_DEFAULT:
			case ResourceType::TRANSIENT_DEFAULT:
				return D3D12_HEAP_TYPE_DEFAULT;
			case ResourceType::PERMANENT_UPLOAD:
			case ResourceType::TRANSIENT_UPLOAD:
				return D3D12_HEAP_TYPE_UPLOAD;
			case ResourceType::PERMANENT_READBACK:
			case ResourceType::TRANSIENT_READBACK:
				return D3D12_HEAP_TYPE_READBACK;
			default:
				return D3D12_HEAP_TYPE_CUSTOM;
			}
		}

		D3D12_RESOURCE_DESC GetDX12Desc() const {
			D3D12_RESOURCE_DESC desc;
			desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			desc.DepthOrArraySize = depth;
			desc.Width = width;
			desc.Height = height;
			desc.Dimension = dimension;
			desc.Flags = flags;
			desc.Format = format;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.MipLevels = mipLevels;
			desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			return desc;
		}
	};

	class ResourcePool {

		struct HeapAlloc {

			com_ptr<ID3D12Heap> heap;

			struct ResourceAlloc {
				UINT64 sizeInBytes;
				com_ptr<ID3D12Resource> resource;

				ResourceAlloc() = default;
				~ResourceAlloc() = default;
				ResourceAlloc(ResourceAlloc &&) = default;
				ResourceAlloc & operator=(ResourceAlloc &&) = default;

				ResourceAlloc(UINT64 size, com_ptr<ID3D12Resource> res) : sizeInBytes{ size }, resource{ res } { }
			};

			std::list<ResourceAlloc> used;
			std::set<ResourceAlloc> freed;

			D3D12_HEAP_TYPE type;

			UINT64 sizeInBytes;
			UINT64 usedInBytes;

			HeapAlloc() : heap{}, used{ }, freed{ }, type{ }, sizeInBytes{ 0 }, usedInBytes{ 0 } { }

			HeapAlloc(ID3D12Device * device, UINT64 sizeInBytes, D3D12_HEAP_TYPE type) : HeapAlloc() {
				this->sizeInBytes = sizeInBytes;
				this->type = type;

				D3D12_HEAP_DESC heapDesc = {};
				heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
				heapDesc.Flags = D3D12_HEAP_FLAG_NONE;
				heapDesc.Properties = CD3DX12_HEAP_PROPERTIES(type);
				heapDesc.SizeInBytes = sizeInBytes;

				DX_API("Failed to create heap")
					device->CreateHeap(&heapDesc, IID_PPV_ARGS(heap.GetAddressOf()));
			}

			HRESULT ReleaseResource(ID3D12Resource * resource) {
				auto it = std::find_if(std::begin(used), std::end(used), [resource](const ResourceAlloc & listItem) -> bool {
					return listItem.resource.Get() == resource;
				});

				if(it == std::end(used)) {
					return E_FAIL;
				}

				it->resource.Reset();
				freed.insert(*it);
				used.erase(it);

				return S_OK;
			}

			ID3D12Resource * CreateResource(ID3D12Device * device, const D3D12_RESOURCE_DESC & desc, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE * clearValue, const D3D12_RESOURCE_ALLOCATION_INFO & allocation) {
				com_ptr<ID3D12Resource> tempResource;

				decltype(freed)::iterator reuseableResource = std::min_element(std::begin(freed), std::end(freed), [allocation](const ResourceAlloc & res) -> bool {
					return res.sizeInBytes >= allocation.SizeInBytes;
				});
				
				DX_API("Failed to create placed resource")
					device->CreatePlacedResource(heap.Get(), usedInBytes, &desc, initState, clearValue, IID_PPV_ARGS(tempResource.GetAddressOf()));

				ID3D12Resource * rawPtr = tempResource.Get();

				usedInBytes += allocation.SizeInBytes;

				used.emplace_back(allocation.SizeInBytes, std::move(tempResource));

				return rawPtr;
			}

			bool CanFit(UINT64 sizeInBytes) const {
				UINT64 totalFreeSpace = (this->sizeInBytes - this->usedInBytes);

				if(totalFreeSpace < sizeInBytes) {
					return false;
				}

				for(const auto & iter : freed) {
					if(iter.sizeInBytes >= sizeInBytes) {
						return true;
					} else {
						totalFreeSpace -= iter.sizeInBytes;
					}
				}

				return totalFreeSpace < sizeInBytes;
			}
		};

		template<UINT64 GRANULARITY>
		class HeapCollection {
			using HeapsType = std::list<HeapAlloc>;

			void SortHeaps() {
				heaps.sort([](const HeapAlloc & lhs, const HeapAlloc & rhs) -> bool {
					return lhs.sizeInBytes > rhs.sizeInBytes;
				});
			}
		public:
			HeapsType heaps;

			D3D12_HEAP_TYPE  heapType;

			HeapCollection(D3D12_HEAP_TYPE heapType) : heapType{ heapType } { }

			

			ID3D12Resource * CreateResource(ID3D12Device* device, const D3D12_RESOURCE_DESC & desc, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE * clearValue) {
				ASSERT(GRANULARITY < alloc.SizeInBytes, "Cant allocate %ull size while the granularity is just %ull", alloc.SizeInBytes, GRANULARITY);

				const D3D12_RESOURCE_ALLOCATION_INFO alloc = device->GetResourceAllocationInfo(0, 1, &desc);

				for(auto & heap : heaps) {
					if(heap.CanFit(alloc.SizeInBytes)) {
						return heap.CreateResource(device, desc, flags, state, clearValue, alloc);
					}
				}

				auto & heap = heaps.emplace_back(device, GRANULARITY, heapType);

				ID3D12Resource * rawPtr = heap.CreateResource(device, desc, flags, state, clearValue, alloc);

				SortHeaps();

				return rawPtr;
			}

			HRESULT ReleaseResource(ID3D12Resource * resource) {
				for(auto & heap : heaps) {
					if(SUCCEEDED(heap.ReleaseResource(resource))) {
						return S_OK;
					}
				}
				return E_FAIL;
			}
		};

		constexpr static UINT64 ALLOCATION_GRANULARITY = 1 << 25; // 32M
		using HeapCollectionType = HeapCollection<ALLOCATION_GRANULARITY>;


		HeapCollectionType defaultPermanentHeaps;
		HeapCollectionType uploadPermanentHeaps;
		HeapCollectionType readbackPermanentHeaps;

		HeapCollectionType defaultTransientHeaps;
		HeapCollectionType uploadTransientHeaps;
		HeapCollectionType readbackTransientHeaps;

		UINT64 totalSizeInBytes;
		UINT64 freeSizeInBytes;

		ID3D12Device * device;

		int nextHandle;
		std::queue<int> availableHandles;
		std::map<int, ResourceDesc> managedResources;


		HeapCollectionType & GetCollection(ResourceType resType) {

			switch(resType) {
			case ResourceType::PERMANENT_DEFAULT:
				return defaultPermanentHeaps;
			case ResourceType::PERMANENT_UPLOAD:
				return uploadPermanentHeaps;
			case ResourceType::PERMANENT_READBACK:
				return readbackPermanentHeaps;
			case ResourceType::TRANSIENT_DEFAULT:
				return defaultTransientHeaps;
			case ResourceType::TRANSIENT_UPLOAD:
				return uploadTransientHeaps;
			case ResourceType::TRANSIENT_READBACK:
				return readbackTransientHeaps;
			}

			throw std::exception("Unexpected control flow");
		}

	public:
		int CreateResource(const ResourceDesc & resource) {
			int idx = -1;

			if(!availableHandles.empty()) {
				idx = availableHandles.front();
				availableHandles.pop();
			} else {
				idx = nextHandle;
				nextHandle += 1;
			}

			ASSERT(managedResources.find(idx) == managedResources.end(), "Trying to replace already existing resource handle: %d", idx);


			HeapCollectionType & heapCollection = GetCollection(resource.type);
			
			const D3D12_CLEAR_VALUE * cv = (resource.dimension != D3D12_RESOURCE_DIMENSION_BUFFER) ? &resource.clearValue : nullptr;

			ID3D12Resource* resPtr = heapCollection.CreateResource(device, resource.GetDX12Desc(), resource.flags, resource.state, cv);

			ResourceDesc mutableResource = resource;
			mutableResource.resource = resPtr;
			mutableResource.bufferLocation = resPtr->GetGPUVirtualAddress();

			managedResources[idx] = mutableResource;

			return idx;
		}

		int CreateStructuredBuffer(ResourceType type, UINT64 sizeInBytes, UINT strideInBytes, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_FLAGS optFlags = D3D12_RESOURCE_FLAG_NONE) {
			ResourceDesc rDesc;
			rDesc.type = type;
			rDesc.dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			rDesc.height = 1;
			rDesc.mipLevels = 0;
			rDesc.sizeInBytes = rDesc.width = sizeInBytes;
			rDesc.strideInBytes = strideInBytes;
			rDesc.format = DXGI_FORMAT_UNKNOWN;
			rDesc.depth = 1;
			rDesc.flags = optFlags;
			rDesc.state = initialState;

			return CreateResource(rDesc);
		}

		int CreateTypedBuffer(ResourceType type, UINT64 sizeInBytes, DXGI_FORMAT format, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_FLAGS optFlags = D3D12_RESOURCE_FLAG_NONE) {
			ResourceDesc rDesc;
			rDesc.type = type;
			rDesc.dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			rDesc.height = 1;
			rDesc.mipLevels = 0;
			rDesc.sizeInBytes = rDesc.width = sizeInBytes;
			rDesc.strideInBytes = DirectX::BitsPerPixel(format) / 8U;
			rDesc.format = format;
			rDesc.depth = 1;
			rDesc.flags = optFlags;
			rDesc.state = initialState;

			return CreateResource(rDesc);
		}



		int CreateDepthBuffer(ResourceType type, UINT width, UINT height, DXGI_FORMAT format, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_FLAGS optFlags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
			ResourceDesc rDesc;
			rDesc.type = type;
			rDesc.dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			rDesc.height = height;
			rDesc.width = static_cast<UINT64>(width);
			rDesc.depth = 1;
			rDesc.mipLevels = 0;
			rDesc.strideInBytes = DirectX::BitsPerPixel(format) / 8U;
			rDesc.sizeInBytes = rDesc.width * rDesc.height * rDesc.strideInBytes;
			rDesc.format = format;
			rDesc.flags = optFlags;
			rDesc.state = initialState;

			bool isFormatValid = (format == DXGI_FORMAT_D16_UNORM ||
								 format == DXGI_FORMAT_D32_FLOAT ||
								 format == DXGI_FORMAT_D24_UNORM_S8_UINT ||
								 format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT);

			ASSERT(isFormatValid, "Depth buffer format is invalid");

			bool hasDepthFlag = (optFlags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0;

			ASSERT(hasDepthFlag, "Depth buffer must have D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL flag set");

			bool isTypeValid = (type == ResourceType::PERMANENT_DEFAULT || type == ResourceType::TRANSIENT_DEFAULT);

			ASSERT(isTypeValid, "Depth buffer must be present in the default heap");

			return CreateResource(rDesc);
		}
		
		const ResourceDesc & GetResourceDesc(int handle) {
			decltype(managedResources)::const_iterator it = managedResources.find(handle);

			ASSERT(it != managedResources.end(), "Could not find resource desc with handle: %d", handle);

			return it->second;
		}

		void ReleaseResource(int handle) {
			decltype(managedResources)::iterator it = managedResources.find(handle);

			ASSERT(it != managedResources.end(), "Could not find resource desc with handle: %d", handle);

			HeapCollectionType & heapCollection = GetCollection(it->second.type);
				
			DX_API("Failed to release resource")
				heapCollection.ReleaseResource(it->second.resource);

			managedResources.erase(it);
			
			availableHandles.push(handle);
		}

	};



}
