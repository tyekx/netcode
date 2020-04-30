#include "DX12HeapManager.h"

namespace Netcode::Graphics::DX12 {

	size_t HeapManager::GetBucketSize(size_t size) {
		constexpr static size_t s512K = 1 << 19;
		constexpr static size_t s4M = 1 << 22;
		constexpr static size_t s64M = 1 << 26;

		if(size <= s512K) {
			return s512K;
		}

		if(size <= s4M) {
			return s4M;
		}

		if(size <= s64M) {
			return s64M;
		}

		return size;
	}
	
	void HeapManager::SetDevice(com_ptr<ID3D12Device> dev) {
		device = std::move(dev);
	}

	D3D12_HEAP_FLAGS GetHeapFlags(D3D12_RESOURCE_FLAGS f, D3D12_RESOURCE_DIMENSION dim) {
		if((f & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) || (f & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) {
			return D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
		}

		if(dim == D3D12_RESOURCE_DIMENSION_BUFFER) {
			return D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		}

		return D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
	}

	DX12ResourceRef HeapManager::CreateResource(const ResourceDesc & d) {
		using value_type = decltype(heaps)::value_type;

		const D3D12_RESOURCE_DESC dxDesc = GetNativeDesc(d);
		const D3D12_RESOURCE_ALLOCATION_INFO dxAlloc = device->GetResourceAllocationInfo(0, 1, &dxDesc);

		ResourceDesc cpy = d;

		if(cpy.sizeInBytes == 0) {
			cpy.sizeInBytes = dxAlloc.SizeInBytes;
		}

		const size_t bucketSize = GetBucketSize(dxAlloc.SizeInBytes);
		const D3D12_RESOURCE_STATES initState = GetNativeState(cpy.state);
		const D3D12_HEAP_TYPE heapType = GetNativeHeapType(cpy.type);
		const D3D12_HEAP_FLAGS heapFlags = GetHeapFlags(GetNativeFlags(cpy.flags), GetNativeDimension(cpy.dimension));

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

		decltype(heaps)::iterator it = std::find_if(std::begin(heaps), std::end(heaps), [&](value_type & item) -> bool {
			if(!item->IsCompatible(heapType, heapFlags)) {
				return false;
			}

			if(!item->HasEnoughSpace(dxAlloc.SizeInBytes)) {
				item->Defragment();
				return item->HasEnoughSpace(dxAlloc.SizeInBytes);
			}

			return true;
		});

		DX12ResourceRef resource;
		if(it != std::end(heaps)) {
			resource = (*it)->CreateResource(cpy, dxDesc, initState, optCv, dxAlloc);
		} else {
			value_type& newHeap = heaps.emplace_back(std::make_shared<Heap>(device, bucketSize, heapType, heapFlags));
			resource = newHeap->CreateResource(cpy, dxDesc, initState, optCv, dxAlloc);
		}

		std::sort(std::begin(heaps), std::end(heaps), [](const value_type & lhs, const value_type & rhs) -> bool {
			return lhs->GetUnallocatedSize() < rhs->GetUnallocatedSize();
		});

		return resource;
	}
}
