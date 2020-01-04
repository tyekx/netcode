#pragma once

#include "DX12Common.h"
#include <list>
#include <set>
#include <algorithm>
#include <unordered_map>

namespace Egg::Graphics::DX12 {

	struct HeapAlloc {

		com_ptr<ID3D12Heap> heap;

		struct ResourceAlloc {
			UINT64 sizeInBytes;
			UINT64 heapOffset;
			com_ptr<ID3D12Resource> resource;

			ResourceAlloc() = default;
			~ResourceAlloc() = default;

			ResourceAlloc(const ResourceAlloc &) = default;
			ResourceAlloc & operator=(const ResourceAlloc &) = default;

			ResourceAlloc(ResourceAlloc &&) = default;
			ResourceAlloc & operator=(ResourceAlloc &&) = default;

			ResourceAlloc(UINT64 size, UINT64 heapOffset, com_ptr<ID3D12Resource> res) : sizeInBytes{ size }, heapOffset{ heapOffset }, resource{ std::move(res) } { }

			bool operator<(const ResourceAlloc & ra) const {
				return sizeInBytes < ra.sizeInBytes;
			}
		};

		std::list<ResourceAlloc> used;
		std::multiset<ResourceAlloc> freed;

		D3D12_HEAP_TYPE type;

		UINT64 sizeInBytes;
		UINT64 usedInBytes;

		bool operator<(const HeapAlloc & rhs) const {
			return usedInBytes < rhs.usedInBytes;
		}

		HeapAlloc(HeapAlloc && o) = default;
		HeapAlloc & operator=(HeapAlloc && o) = default;

		HeapAlloc(const HeapAlloc & o) = delete;
		HeapAlloc & operator=(const HeapAlloc & o) = delete;

		HeapAlloc() : heap{}, used{ }, freed{ }, type{ }, sizeInBytes{ 0 }, usedInBytes{ 0 } { }

		HeapAlloc(ID3D12Device * device, UINT64 sizeInBytes, D3D12_HEAP_TYPE type, D3D12_HEAP_FLAGS flags) : HeapAlloc() {
			this->sizeInBytes = sizeInBytes;
			this->type = type;

			D3D12_HEAP_DESC heapDesc = {};
			heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			heapDesc.Flags = flags;
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

		ID3D12Resource * CreateResource(ID3D12Device * device, const D3D12_RESOURCE_DESC & desc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE * clearValue, const D3D12_RESOURCE_ALLOCATION_INFO & allocation) {
			com_ptr<ID3D12Resource> tempResource;

			/*
			partition point: returns the element for which the predicate returns false.
			Keeping it in a multiset enables us to have a sorted collection of freed allocations
			idx:  |   0    1    2     3  |
			pred: | true true true false |
			      |                   ^ this will be the partition point
			if no partiton point found end() will be returned
			*/
			decltype(freed)::iterator reuseableResource = std::partition_point(std::begin(freed), std::end(freed), [allocation](const ResourceAlloc & res) -> bool {
				return res.sizeInBytes < allocation.SizeInBytes;
			});

			UINT64 heapOffset;
			UINT64 allocationSize;

			if(reuseableResource == freed.end()) {
				heapOffset = usedInBytes;
				allocationSize = allocation.SizeInBytes;
			} else {
				heapOffset = reuseableResource->heapOffset;
				allocationSize = reuseableResource->sizeInBytes;
			}

			DX_API("Failed to create placed resource")
				device->CreatePlacedResource(heap.Get(), heapOffset, &desc, initState, clearValue, IID_PPV_ARGS(tempResource.GetAddressOf()));

			ID3D12Resource * rawPtr = tempResource.Get();

			used.emplace_back(allocationSize, heapOffset, std::move(tempResource));
			if(reuseableResource != freed.end()) {
				freed.erase(reuseableResource);
			}

			usedInBytes += allocationSize;

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

	class HeapManager {

		static uint32_t DeduceBucketIndex(size_t size) {
			constexpr static size_t s512K = 1 << 19;
			constexpr static size_t s4M = 1 << 22;
			constexpr static size_t s32M = 1 << 25;

			if(size <= s512K) {
				return 0;
			}

			if(size <= s4M) {
				return 1;
			}

			if(size <= s32M) {
				return 2;
			}

			return 3;
		}

		static size_t GetBucketSize(size_t size) {
			constexpr static size_t s512K = 1 << 19;
			constexpr static size_t s4M = 1 << 22;
			constexpr static size_t s32M = 1 << 25;

			if(size <= s512K) {
				return s512K;
			}

			if(size <= s4M) {
				return s4M;
			}

			if(size <= s32M) {
				return s32M;
			}

			return size;
		}

		struct ResourceHash {
			uint32_t hash;

			uint32_t GetHeapTypeBits() const {
				return (hash >> 6) & 0x3;
			}

			uint32_t GetHeapFlagBits() const {
				return (hash >> 4) & 0x3;
			}

			void SetHeapTypeBits(D3D12_HEAP_TYPE type) {
				uint32_t bits;
				switch(type) {
					case D3D12_HEAP_TYPE_DEFAULT: bits = 0; break;
					case D3D12_HEAP_TYPE_UPLOAD: bits = 1; break;
					case D3D12_HEAP_TYPE_READBACK: bits = 2; break;
					default: bits = 3; break;
				}
				hash |= (bits) << 6;
			}

			void SetHeapFlagBits(D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_DIMENSION dim) {
				uint32_t heapFlagBits = 0;
				if((flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) ||
					(flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) {
					heapFlagBits |= 1u;
				}

				if(dim == D3D12_RESOURCE_DIMENSION_BUFFER) {
					heapFlagBits |= 2u;
				}
				hash |= (heapFlagBits & 0x3) << 4;
			}

			void SetBucketIndex(uint32_t bits) {
				hash |= (bits & 0xF);
			}

			void SetTransientBit(bool isTransient) {
				if(isTransient) {
					hash |= (0x1 << 8);
				}
			}
		public:
			ResourceHash & operator=(const ResourceHash &) = default;
			ResourceHash(const ResourceHash &) = default;

			ResourceHash(const Egg::Graphics::ResourceDesc & rd) : hash{ 0 } {
				SetHeapFlagBits(GetNativeFlags(rd.flags), GetNativeDimension(rd.dimension));
				SetHeapTypeBits(GetNativeHeapType(rd.type));
				SetBucketIndex(HeapManager::DeduceBucketIndex(rd.sizeInBytes));
				bool isTransient = (rd.type == ResourceType::TRANSIENT_DEFAULT || rd.type == ResourceType::TRANSIENT_UPLOAD || rd.type == ResourceType::TRANSIENT_READBACK);
				SetTransientBit(isTransient);
			}

			bool operator==(const ResourceHash & h) const {
				return hash == h.hash;
			}

			bool operator<(const ResourceHash & h) const {
				return hash < h.hash;
			}

			bool IsTransient() const {
				return (hash & 256u) > 0;
			}

			bool IsPermanent() const {
				return !IsTransient();
			}

			bool IsRenderTarget() const {
				return (hash & 16u) > 0;
			}

			bool IsBuffer() const {
				return (hash & 32u) > 0;
			}

			bool IsTexture() const {
				return !IsBuffer();
			}

			uint32_t GetHash() const {
				return hash;
			}

			uint32_t GetBucketIndex() const {
				return hash & (0xF);
			}

			D3D12_HEAP_FLAGS GetHeapFlag() const {
				switch(GetHeapFlagBits()) {
					case 0: // not buffer and not render target
						return D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
					case 1: // not buffer but render target
						return D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
					case 2: // buffer
						return D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
					default: // unexpected case
						return D3D12_HEAP_FLAG_NONE;
				}
			}

			D3D12_HEAP_TYPE GetHeapType() const {
				switch(GetHeapTypeBits()) {
					case 0: return D3D12_HEAP_TYPE_DEFAULT;
					case 1: return D3D12_HEAP_TYPE_UPLOAD;
					case 2: return D3D12_HEAP_TYPE_READBACK;
					default: return D3D12_HEAP_TYPE_CUSTOM;
				}
			}
		};


		template<typename T>
		struct AlreadyHashed {
			size_t operator()(const T & h) const {
				return h.GetHash();
			}
		};

		std::unordered_map<ResourceHash, std::list<HeapAlloc>, AlreadyHashed<ResourceHash>> collections;
		std::unordered_map<ID3D12Resource *, ResourceHash> hashAssoc;



		ID3D12Device * device;

		inline ID3D12Resource * MarshalResource(ID3D12Resource * resource, const ResourceHash & hash) {
			hashAssoc.emplace(resource, hash);
			return resource;
		}
	public:
		void SetDevice(ID3D12Device * device) {
			this->device = device;
		}

		ID3D12Resource* CreateResource(const ResourceDesc & d) {
			const D3D12_RESOURCE_DESC dxDesc = GetNativeDesc(d);
			const D3D12_RESOURCE_STATES initState = GetNativeState(d.state);
			const ResourceHash hash(d);
			const D3D12_HEAP_TYPE heapType = hash.GetHeapType();
			const D3D12_HEAP_FLAGS heapFlags = hash.GetHeapFlag();
			const D3D12_RESOURCE_ALLOCATION_INFO dxAlloc = device->GetResourceAllocationInfo(0, 1, &dxDesc);
			const size_t bucketSize = GetBucketSize(dxAlloc.SizeInBytes);

			decltype(collections)::iterator collection = collections.find(hash);

			if(collection != collections.end()) {
				
				for(HeapAlloc & heap : collection->second) {
					if(heap.CanFit(dxAlloc.SizeInBytes)) {
						return MarshalResource(heap.CreateResource(device, dxDesc, initState, nullptr, dxAlloc), hash);
					}
				}

			} else {
				collections[hash];
				collection = collections.find(hash);

				ASSERT(collection != collections.end(), "oof");
			}
			
			HeapAlloc & heapAlloc = collection->second.emplace_back(device, bucketSize, heapType, heapFlags);
			return MarshalResource(heapAlloc.CreateResource(device, dxDesc, initState, nullptr, dxAlloc), hash);
		}

		void ReleaseResource(ID3D12Resource * resource) {
			auto it = hashAssoc.find(resource);

			if(it != hashAssoc.end()) {

				auto & collection = collections[it->second];

				for(HeapAlloc & heap : collection) {
					if(SUCCEEDED(heap.ReleaseResource(resource))) {
						break;
					}
				}

				Log::Warn("You broke it");

				hashAssoc.erase(it);

			} else Log::Info("Skipping resource deallocation");
		}

	};


}
