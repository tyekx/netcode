#include "DX12Resource.h"
#include "DX12Heap.h"

namespace Netcode::Graphics::DX12 {

	const ResourceDesc & Resource::GetDesc() const {
		return desc;
	}

	Resource::Resource(uint64_t sizeInBytes,
		uint64_t heapOffset,
		const ResourceDesc & resourceDesc,
		const D3D12_RESOURCE_DESC & nativeDesc,
		com_ptr<ID3D12Resource> resource,
		Ref<Heap> heap) :
		sizeInBytes{ sizeInBytes }, heapOffset{ heapOffset }, desc{ resourceDesc }, nativeDesc{ nativeDesc }, resource{ std::move(resource) }, heap{ std::move(heap) } {

	}

}
