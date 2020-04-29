#include "DX12HeapManager.h"

namespace Netcode::Graphics::DX12 {

	uint32_t HeapManager::DeduceBucketIndex(size_t size) {
		constexpr static size_t s256K = 1 << 18;
		constexpr static size_t s2M = 1 << 21;
		constexpr static size_t s32M = 1 << 25;

		if(size <= s256K) {
			return 0;
		}

		if(size <= s2M) {
			return 1;
		}

		if(size <= s32M) {
			return 2;
		}

		return 3;
	}

	size_t HeapManager::GetBucketSize(size_t size) {
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
	
	void HeapManager::SetDevice(com_ptr<ID3D12Device> dev) {
		device = std::move(dev);
	}

	DX12ResourceRef HeapManager::CreateResource(const ResourceDesc & d) {
		const D3D12_RESOURCE_DESC dxDesc = GetNativeDesc(d);
		const D3D12_RESOURCE_ALLOCATION_INFO dxAlloc = device->GetResourceAllocationInfo(0, 1, &dxDesc);

		ResourceDesc cpy = d;

		if(cpy.sizeInBytes == 0) {
			cpy.sizeInBytes = dxAlloc.SizeInBytes;
		}

		const size_t bucketSize = GetBucketSize(dxAlloc.SizeInBytes);
		const D3D12_RESOURCE_STATES initState = GetNativeState(cpy.state);
		const ResourceHash hash(cpy);
		const D3D12_HEAP_TYPE heapType = hash.GetHeapType();
		const D3D12_HEAP_FLAGS heapFlags = hash.GetHeapFlag();

		decltype(collections)::iterator collection = collections.find(hash);

		D3D12_CLEAR_VALUE cv;
		D3D12_CLEAR_VALUE * optCv = nullptr;

		if((dxDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) > 0) {
			cv.Color[0] = 0.0f;
			cv.Color[1] = 0.0f;
			cv.Color[2] = 0.0f;
			cv.Color[3] = 0.0f;
			cv.Format = dxDesc.Format;
			optCv = &cv;
		} else if((dxDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) > 0) {
			cv.DepthStencil.Depth = 1.0f;
			cv.DepthStencil.Stencil = 0;
			cv.Format = dxDesc.Format;
			optCv = &cv;
		}

		if(collection != collections.end()) {
			for(std::shared_ptr<Heap> & heap : collection->second) {
				if(heap->HasEnoughSpace(dxAlloc.SizeInBytes)) {
					return heap->CreateResource(cpy, dxDesc, initState, optCv, dxAlloc);
				}
			}
		} else {
			collections[hash];
			collection = collections.find(hash);

			ASSERT(collection != collections.end(), "oof");
		}

		auto heap = std::make_shared<Heap>(device, bucketSize, heapType, heapFlags);

		collection->second.emplace_back(heap);

		return heap->CreateResource(cpy, dxDesc, initState, optCv, dxAlloc);
	}
}
