#pragma once

#include "DX12Common.h"
#include "DX12ResourceDesc.h"
#include "DX12HeapCollection.h"

#include <DirectXTex/DirectXTex.h>


namespace Netcode::Graphics::DX12 {

	class ResourcePool {

		HeapManager * heapManager;

		std::list<GResource> managedResources;

	public:

		void SetHeapManager(HeapManager * heapMan) {
			heapManager = heapMan;
		}

		uint64_t CreateResource(const ResourceDesc & resource);

		uint64_t CreateStructuredBuffer(size_t sizeInBytes, UINT strideInBytes, ResourceType type, ResourceState initialState, ResourceFlags flags);

		uint64_t CreateTypedBuffer(size_t sizeInBytes, DXGI_FORMAT format, ResourceType type, ResourceState initialState, ResourceFlags flags);

		uint64_t CreateDepthBuffer(ResourceType type, UINT width, UINT height, DXGI_FORMAT format, ResourceState initialState = ResourceState::DEPTH_WRITE, ResourceFlags optFlags = ResourceFlags::ALLOW_DEPTH_STENCIL);
		
		const ResourceDesc & GetResourceDesc(uint64_t handle);

		const GResource & GetNativeResource(uint64_t handle);

		void ReleaseResource(uint64_t handle);

		void ReleaseTransients();
	};


}
