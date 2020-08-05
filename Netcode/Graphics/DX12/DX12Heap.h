#pragma once

#include <Netcode/HandleDecl.h>
#include "DX12Decl.h"
#include <memory>
#include <vector>
#include <wrl.h>

namespace Netcode::Graphics::DX12 {

	class Resource;

	class Heap : public std::enable_shared_from_this<Heap> {
		uint64_t sizeInBytes;
		uint64_t offsetInBytes;
		uint64_t freedSizeInBytes;
		D3D12_HEAP_TYPE type;
		D3D12_HEAP_FLAGS flags;
		com_ptr<ID3D12Device> device;
		com_ptr<ID3D12Heap> heap;
		std::vector<Unique<Resource>> freedResources;

		void ReturnResource(Resource * rawPtr);

	public:
		Heap(com_ptr<ID3D12Device> dev, uint64_t sizeInBytes, D3D12_HEAP_TYPE type, D3D12_HEAP_FLAGS flags);

		bool IsCompatible(D3D12_HEAP_TYPE requiredHeap, D3D12_HEAP_FLAGS requiredFlags) const;

		bool HasEnoughSpace(uint64_t bytesToStore) const;

		bool IsEmpty() const;

		uint64_t GetUnallocatedSize() const;

		void Defragment();

		Ref<Resource> CreateResource(
			const ResourceDesc & resourceDesc, 
			const D3D12_RESOURCE_DESC & desc,
			D3D12_RESOURCE_STATES initState,
			const D3D12_CLEAR_VALUE * clearValue,
			const D3D12_RESOURCE_ALLOCATION_INFO & allocation);
	};

}
