#include "DX12UploadBatch.h"

namespace Netcode::Graphics::DX12 {

	BarrierTask::BarrierTask(Ref<GpuResource> handle, ResourceState before, ResourceState after) : resourceHandle{ handle }, before{ before }, after{ after } { }

	BufferUploadTask::BufferUploadTask(Ref<GpuResource> h, const void * srcData, size_t srcDataSizeInBytes) : BufferUploadTask(std::move(h), srcData, srcDataSizeInBytes, 0) {

	}

	BufferUploadTask::BufferUploadTask(Ref<GpuResource> h, const void * srcData, size_t srcDataSizeInBytes, size_t dstDataOffsetInBytes) : resourceHandle{ std::move(h) }, srcData{ srcData }, srcDataSizeInBytes{ srcDataSizeInBytes }, dstDataOffsetInBytes{ dstDataOffsetInBytes }
	{

	}

	BufferUploadTask::BufferUploadTask() : resourceHandle{ 0 }, srcData{ nullptr }, srcDataSizeInBytes{ 0 }, dstDataOffsetInBytes{ 0 } {

	}

	TextureUploadTask::TextureUploadTask() : resourceHandle{ 0 }, texture{ nullptr } {

	}

	TextureUploadTask::TextureUploadTask(Ref<GpuResource> h, Ref<Texture> texture) : resourceHandle{ h }, texture{ std::move(texture) } {

	}

	void UploadBatch::Upload(Ref<GpuResource> resourceHandle, const void * srcData, size_t srcDataSizeInBytes) {
		uploadTasks.emplace_back(BufferUploadTask(resourceHandle, srcData, srcDataSizeInBytes));
	}

	void UploadBatch::Upload(Ref<GpuResource> resourceHandle, const void * srcData, size_t srcDataSizeInBytes, size_t dstDataOffsetInBytes)
	{
		uploadTasks.emplace_back(BufferUploadTask(resourceHandle, srcData, srcDataSizeInBytes, dstDataOffsetInBytes));
	}

	void  UploadBatch::Upload(Ref<GpuResource> resourceHandle, Ref<Texture> texture) {
		uploadTasks.emplace_back(TextureUploadTask(resourceHandle, std::move(texture)));
	}

	void UploadBatch::ResourceBarrier(Ref<GpuResource> resourceHandle, ResourceState before, ResourceState after) {
		barrierTasks.emplace_back(resourceHandle, before, after);
	}

	const std::vector<UploadTask> & UploadBatch::UploadTasks() const {
		return uploadTasks;
	}

	const std::vector<BarrierTask> UploadBatch::BarrierTasks() const {
		return barrierTasks;
	}

}
