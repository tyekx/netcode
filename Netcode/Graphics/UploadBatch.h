#pragma once

#include "../Common.h"
#include "../HandleTypes.h"
#include "ResourceEnums.h"
#include <vector>

namespace Egg::Graphics {

	enum class UploadTaskType : unsigned {
		TEXTURE, BUFFER
	};


	class UploadBatch {

	public:
		void Upload(uint64_t resourceHandle, Egg::TextureRef texture);

		void Upload(uint64_t resourceHandle, const void * srcData, size_t srcDataSizeInBytes);

		void ResourceBarrier(uint64_t resourceHandle, ResourceState before, ResourceState after);

		struct BufferUploadTask {
			const uint64_t resourceHandle;
			const void * srcData;
			const size_t srcDataSizeInBytes;

			BufferUploadTask();

			BufferUploadTask(uint64_t h, const void * srcData, size_t srcDataSizeInBytes);
		};

		struct TextureUploadTask {
			const uint64_t resourceHandle;
			const Egg::TextureRef texture;

			TextureUploadTask();

			TextureUploadTask(const TextureUploadTask &) = default;

			TextureUploadTask(uint64_t h, Egg::TextureRef texture);
		};

		struct UploadTask {
			TextureUploadTask textureTask;
			BufferUploadTask bufferTask;
			UploadTaskType type;

			UploadTask(const TextureUploadTask & textureTask) : textureTask{ textureTask }, bufferTask{}, type { UploadTaskType::TEXTURE } { }
			UploadTask(const BufferUploadTask & bufferTask) : textureTask{  }, bufferTask{ bufferTask }, type { UploadTaskType::BUFFER } { }
		};

		struct BarrierTask {
			const uint64_t resourceHandle;
			const ResourceState before;
			const ResourceState after;
			
			BarrierTask(const BarrierTask &) = default;
			BarrierTask(uint64_t handle, ResourceState before, ResourceState after);
		};

		const std::vector<UploadTask> & UploadTasks() const;

		const std::vector<BarrierTask> BarrierTasks() const;

	private:
		std::vector<UploadTask> uploadTasks;
		std::vector<BarrierTask> barrierTasks;
	};

}
