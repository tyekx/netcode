#pragma once

#include "DX12Common.h"
#include <list>
#include <set>
#include <algorithm>
#include <unordered_map>

#include "DX12ResourceDesc.h"
#include "DX12Resource.h"
#include "DX12Heap.h"

namespace Netcode::Graphics::DX12 {

	class HeapManager {
		static uint32_t DeduceBucketIndex(size_t size);

		static size_t GetBucketSize(size_t size);

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

			ResourceHash(const Netcode::Graphics::ResourceDesc & rd) : hash{ 0 } {
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

		std::unordered_map<ResourceHash, std::list<std::shared_ptr<Heap>>, AlreadyHashed<ResourceHash>> collections;

		com_ptr<ID3D12Device> device;
	public:
		void SetDevice(com_ptr<ID3D12Device> dev);

		DX12ResourceRef CreateResource(const ResourceDesc & d);
	};

	using DX12HeapManager = Netcode::Graphics::DX12::HeapManager;
	using DX12HeapManagerRef = std::shared_ptr<DX12HeapManager>;

}
