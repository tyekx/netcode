#pragma once

#include <Netcode/HandleDecl.h>

namespace Netcode::Graphics {

	class UploadBatch {
	public:
		virtual ~UploadBatch() = default;

		virtual void Upload(Ref<GpuResource> resourceHandle, Ref<Texture> texture) = 0;
		virtual void Upload(Ref<GpuResource> resourceHandle, const void * srcData, size_t srcDataSizeInBytes) = 0;
		virtual void Upload(Ref<GpuResource> resourceHandle, const void * srcData, size_t srcDataSizeInBytes, size_t dstDataOffsetInBytes) = 0;
		virtual void Barrier(Ref<GpuResource> resourceHandle, ResourceState before, ResourceState after) = 0;
	};

}
