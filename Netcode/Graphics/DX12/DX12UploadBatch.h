#pragma once

#include <Netcode/Graphics/UploadBatch.h>
#include <Netcode/HandleTypes.h>

#include <vector>
#include <variant>

namespace Netcode::Graphics::DX12 {

	struct BufferUploadTask {
		const Ref<GpuResource> resourceHandle;
		const void * srcData;
		const size_t srcDataSizeInBytes;
		const size_t dstDataOffsetInBytes;

		BufferUploadTask();
		BufferUploadTask(Ref<GpuResource> h, const void * srcData, size_t srcDataSizeInBytes);
		BufferUploadTask(Ref<GpuResource> h, const void * srcData, size_t srcDataSizeInBytes, size_t dstDataOffsetInBytes);
	};

	struct TextureUploadTask {
		const Ref<GpuResource> resourceHandle;
		const Ref<Texture> texture;

		TextureUploadTask();
		TextureUploadTask(const TextureUploadTask &) = default;
		TextureUploadTask(Ref<GpuResource> h, Ref<Texture> texture);
	};

	using UploadTask = std::variant<BufferUploadTask, TextureUploadTask>;

	struct BarrierTask {
		const Ref<GpuResource> resourceHandle;
		const ResourceState before;
		const ResourceState after;

		BarrierTask(const BarrierTask &) = default;
		BarrierTask(Ref<GpuResource> handle, ResourceState before, ResourceState after);
	};


	class UploadBatch : public Netcode::Graphics::UploadBatch {
	public:
	private:
		std::vector<UploadTask> uploadTasks;
		std::vector<BarrierTask> barrierTasks;

	public:
		virtual void Upload(Ref<GpuResource> resourceHandle, Ref<Texture> texture) override;
		virtual void Upload(Ref<GpuResource> resourceHandle, const void * srcData, size_t srcDataSizeInBytes) override;
		virtual void Upload(Ref<GpuResource> resourceHandle, const void * srcData, size_t srcDataSizeInBytes, size_t dstDataOffsetInBytes) override;
		virtual void Barrier(Ref<GpuResource> resourceHandle, ResourceState before, ResourceState after) override;

		const std::vector<UploadTask> & UploadTasks() const;
		const std::vector<BarrierTask> & BarrierTasks() const;
	};

}
