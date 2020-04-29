#pragma once

#include "../Common.h"
#include "../HandleTypes.h"
#include "ResourceEnums.h"
#include <vector>

namespace Netcode::Graphics {

	enum class UploadTaskType : unsigned {
		TEXTURE, BUFFER
	};


	class UploadBatch {

	public:
		void Upload(GpuResourceRef resourceHandle, Netcode::TextureRef texture);

		void Upload(GpuResourceRef resourceHandle, const void * srcData, size_t srcDataSizeInBytes);

		void ResourceBarrier(GpuResourceRef resourceHandle, ResourceState before, ResourceState after);

		struct BufferUploadTask {
			const GpuResourceRef resourceHandle;
			const void * srcData;
			const size_t srcDataSizeInBytes;

			BufferUploadTask();

			BufferUploadTask(GpuResourceRef h, const void * srcData, size_t srcDataSizeInBytes);
		};

		struct TextureUploadTask {
			const GpuResourceRef resourceHandle;
			const Netcode::TextureRef texture;

			TextureUploadTask();

			TextureUploadTask(const TextureUploadTask &) = default;

			TextureUploadTask(GpuResourceRef h, Netcode::TextureRef texture);
		};

		struct UploadTask {
			TextureUploadTask textureTask;
			BufferUploadTask bufferTask;
			UploadTaskType type;

			UploadTask(const TextureUploadTask & textureTask) : textureTask{ textureTask }, bufferTask{}, type { UploadTaskType::TEXTURE } { }
			UploadTask(const BufferUploadTask & bufferTask) : textureTask{  }, bufferTask{ bufferTask }, type { UploadTaskType::BUFFER } { }
		};

		struct BarrierTask {
			const GpuResourceRef resourceHandle;
			const ResourceState before;
			const ResourceState after;
			
			BarrierTask(const BarrierTask &) = default;
			BarrierTask(GpuResourceRef handle, ResourceState before, ResourceState after);
		};

		const std::vector<UploadTask> & UploadTasks() const;

		const std::vector<BarrierTask> BarrierTasks() const;

	private:
		std::vector<UploadTask> uploadTasks;
		std::vector<BarrierTask> barrierTasks;
	};

}
