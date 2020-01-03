#pragma once

#include "Common.h"
#include "ResourceEnums.h"
#include <vector>

namespace Egg::Graphics {

	class UploadBatch {
	public:
		void Upload(uint64_t resourceHandle, const void * srcData, size_t srcDataSizeInBytes);

		void ResourceBarrier(uint64_t resourceHandle, ResourceState before, ResourceState after);

		struct UploadTask {
			const uint64_t resourceHandle;
			const void * srcData;
			const size_t srcDataSizeInBytes;

			UploadTask(const UploadTask &) = default;
			UploadTask & operator=(const UploadTask &) = default;

			UploadTask(uint64_t h, const void * srcData, size_t srcDataSizeInBytes);
		};

		struct BarrierTask {
			const uint64_t resourceHandle;
			const ResourceState before;
			const ResourceState after;
			
			BarrierTask(const BarrierTask &) = default;
			BarrierTask & operator=(const BarrierTask &) = default;
			BarrierTask(uint64_t handle, ResourceState before, ResourceState after);
		};

		const std::vector<UploadTask> & UploadTasks() const;

		const std::vector<BarrierTask> BarrierTasks() const;

	private:
		std::vector<UploadTask> uploadTasks;
		std::vector<BarrierTask> barrierTasks;
	};

}
