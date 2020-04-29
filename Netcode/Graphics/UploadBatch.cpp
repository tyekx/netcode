#include "UploadBatch.h"

namespace Netcode::Graphics {

	UploadBatch::BarrierTask::BarrierTask(GpuResourceRef handle, ResourceState before, ResourceState after) : resourceHandle{ handle }, before{ before }, after{ after } { }

	UploadBatch::BufferUploadTask::BufferUploadTask(GpuResourceRef h, const void * srcData, size_t srcDataSizeInBytes) : resourceHandle{ h }, srcData{ srcData }, srcDataSizeInBytes{ srcDataSizeInBytes } {
	
	}

	UploadBatch::BufferUploadTask::BufferUploadTask() : resourceHandle{ 0 }, srcData{ nullptr }, srcDataSizeInBytes{ 0 } {
	
	} 

	UploadBatch::TextureUploadTask::TextureUploadTask() : resourceHandle{ 0 }, texture{ nullptr } {
	
	}

	UploadBatch::TextureUploadTask::TextureUploadTask(GpuResourceRef h, Netcode::TextureRef texture) : resourceHandle{ h }, texture{ std::move(texture )} {
	
	}

	void UploadBatch::Upload(GpuResourceRef resourceHandle, const void * srcData, size_t srcDataSizeInBytes) {
		uploadTasks.emplace_back(BufferUploadTask(resourceHandle, srcData, srcDataSizeInBytes));
	}

	void  UploadBatch::Upload(GpuResourceRef resourceHandle, Netcode::TextureRef texture) {
		uploadTasks.emplace_back(TextureUploadTask(resourceHandle, std::move(texture)));
	}

	void UploadBatch::ResourceBarrier(GpuResourceRef resourceHandle, ResourceState before, ResourceState after) {
		barrierTasks.emplace_back(resourceHandle, before, after);
	}

	const std::vector<UploadBatch::UploadTask> & UploadBatch::UploadTasks() const {
		return uploadTasks;
	}

	const std::vector<UploadBatch::BarrierTask> UploadBatch::BarrierTasks() const {
		return barrierTasks;
	}

}

