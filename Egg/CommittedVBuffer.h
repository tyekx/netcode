#pragma once

#include "Resource.h"

namespace Egg::Graphics::Resource::Committed {

	class VBuffer : public AVBuffer {
		com_ptr<ID3D12Resource> resource;
		com_ptr<ID3D12Resource> uploadResource;
		D3D12_RESOURCE_DESC resourceDesc;
	public:
		void SetDesc(const D3D12_RESOURCE_DESC & resDesc);

		void CopyToUploadBuffer(const void * srcData, UINT64 sizeInBytes);

		void SetStride(UINT strideInBytes);

		void CreateResources(ID3D12Device * device, const D3D12_RESOURCE_DESC & desc, const void * srcData, UINT64 sizeInBytes, UINT strideInBytes);

		virtual void CreateResources(ID3D12Device * device)  override;

		virtual void ReleaseResources() override;

		virtual void UploadResources(ID3D12GraphicsCommandList * copyCommandList)  override;

		virtual void ReleaseUploadResources() override;
	};

}
