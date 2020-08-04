#pragma once

#include "DX12Common.h"
#include "DX12ResourceDesc.h"
#include "DX12HeapManager.h"

namespace Netcode::Graphics::DX12 {

	class ResourcePool {
		std::shared_ptr<HeapManager> heapManager;

	public:
		void SetHeapManager(std::shared_ptr<HeapManager> heapMan) {
			heapManager = std::move(heapMan);
		}

		Ref<DX12::Resource> CreateResource(const ResourceDesc & resource);

		Ref<DX12::Resource> CreateStructuredBuffer(size_t sizeInBytes, UINT strideInBytes, ResourceType type, ResourceState initialState, ResourceFlags flags);

		Ref<DX12::Resource> CreateTypedBuffer(size_t sizeInBytes, DXGI_FORMAT format, ResourceType type, ResourceState initialState, ResourceFlags flags);

		Ref<DX12::Resource> CreateDepthBuffer(ResourceType type, UINT width, UINT height, DXGI_FORMAT format, ResourceState initialState = ResourceState::DEPTH_WRITE, ResourceFlags optFlags = ResourceFlags::ALLOW_DEPTH_STENCIL);

	};


}
