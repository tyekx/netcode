#pragma once

#include "DX12Resource.h"
#include <memory>
#include <list>
#include <cstdint>

namespace Netcode::Graphics::DX12 {

	class Heap : public std::enable_shared_from_this<Heap> {
		uint64_t sizeInBytes;
		uint64_t offsetInBytes;
		uint64_t freeSizeInBytes;
		D3D12_HEAP_TYPE type;
		com_ptr<ID3D12Device> device;
		com_ptr<ID3D12Heap> heap;
		std::list<std::unique_ptr<Resource>> freedResources;

		void ReturnResource(Resource * rawPtr);

	public:
		Heap(com_ptr<ID3D12Device> dev, uint64_t sizeInBytes, D3D12_HEAP_TYPE type, D3D12_HEAP_FLAGS flags);

		bool HasEnoughSpace(uint64_t bytesToStore);

		std::shared_ptr<Resource> CreateResource(
			const ResourceDesc & resourceDesc, 
			const D3D12_RESOURCE_DESC & desc,
			D3D12_RESOURCE_STATES initState,
			const D3D12_CLEAR_VALUE * clearValue,
			const D3D12_RESOURCE_ALLOCATION_INFO & allocation);
	};

}
