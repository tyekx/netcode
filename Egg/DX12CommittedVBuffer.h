#pragma once

#include "DX12Resource.h"

namespace Egg::Graphics::DX12::Resource {

	class CommittedVBuffer : public AVBuffer {

		struct LODLevelResources {
			com_ptr<ID3D12Resource> resource;
			const void * srcData;
			UINT64 sizeInBytes;
		};

		std::vector<LODLevelResources> lodLevels;
		UINT stride;

	public:
		void SetStride(UINT strideInBytes);

		void AddLODLevel(ID3D12Device * device, const void * srcData, UINT64 sizeInBytes, UINT strideInBytes);

		virtual void CreateResources(ID3D12Device * device)  override;

		virtual void ReleaseResources() override;

		virtual void UploadResources(IResourceUploader * uploader)  override;

		virtual void ReleaseUploadResources() override;
	};

}
