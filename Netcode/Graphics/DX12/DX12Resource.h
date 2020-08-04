#pragma once

#include "../../HandleTypes.h"
#include "DX12Common.h"

namespace Netcode::Graphics::DX12 { 

	class Heap;

	class Resource : public GpuResource {
	public:
		uint64_t sizeInBytes;
		uint64_t heapOffset;
		ResourceDesc desc;
		D3D12_RESOURCE_DESC nativeDesc;
		com_ptr<ID3D12Resource> resource;
		Ref<Heap> heap;

		virtual const ResourceDesc & GetDesc() const override;

		Resource() = default;
		Resource(const Resource &) = delete;

		Resource(uint64_t sizeInBytes,
			uint64_t heapOffset,
			const ResourceDesc & resourceDesc,
			const D3D12_RESOURCE_DESC & nativeDesc,
			com_ptr<ID3D12Resource> resource,
			Ref<Heap> heap);
	};

}


