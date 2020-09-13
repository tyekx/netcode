#pragma once

#include <Netcode/Graphics/ResourceEnums.h>

#include "DX12Common.h"
#include "DX12ResourceDesc.h"
#include "DX12HeapManager.h"

namespace Netcode::Graphics::DX12 {

	class ResourcePool {
		Ref<HeapManager> heapManager;

	public:
		ResourcePool(Ref<HeapManager> heapMan);

		Ref<Resource> CreateResource(const ResourceDesc & resource);

		Ref<Resource> CreateStructuredBuffer(size_t sizeInBytes, UINT strideInBytes, ResourceType type, ResourceStates initialState, ResourceFlags flags);

		Ref<Resource> CreateTypedBuffer(size_t sizeInBytes, DXGI_FORMAT format, ResourceType type, ResourceStates initialState, ResourceFlags flags);

		Ref<Resource> CreateDepthBuffer(ResourceType type, UINT width, UINT height, DXGI_FORMAT format, ResourceStates initialState = ResourceState::DEPTH_WRITE, ResourceFlags optFlags = ResourceFlags::ALLOW_DEPTH_STENCIL);

	};


}
