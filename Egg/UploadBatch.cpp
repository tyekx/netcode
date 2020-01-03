#include "UploadBatch.h"

namespace Egg::Graphics {

	UploadBatch::BarrierTask::BarrierTask(uint64_t handle, ResourceState before, ResourceState after) : resourceHandle{ handle }, before{ before }, after{ after } {
	
	}

	UploadBatch::UploadTask::UploadTask(uint64_t h, const void * srcData, size_t srcDataSizeInBytes) : resourceHandle{ h }, srcData{ srcData }, srcDataSizeInBytes{ srcDataSizeInBytes } {
	
	}

	void UploadBatch::Upload(uint64_t resourceHandle, const void * srcData, size_t srcDataSizeInBytes) {
		uploadTasks.emplace_back(resourceHandle, srcData, srcDataSizeInBytes);
	}

	void UploadBatch::ResourceBarrier(uint64_t resourceHandle, ResourceState before, ResourceState after) {
		barrierTasks.emplace_back(resourceHandle, before, after);
	}

	const std::vector<UploadBatch::UploadTask> & UploadBatch::UploadTasks() const {
		return uploadTasks;
	}

	const std::vector<UploadBatch::BarrierTask> UploadBatch::BarrierTasks() const {
		return barrierTasks;
	}

}

