#pragma once

#include "Resource.h"

namespace Egg::Graphics::Resource::Committed {

	class IBuffer : public AIBuffer {
		com_ptr<ID3D12Resource> resource;
		com_ptr<ID3D12Resource> uploadResource;
		D3D12_RESOURCE_DESC resourceDesc;
	public:
		void SetDesc(const D3D12_RESOURCE_DESC & resDesc);

		void CopyToUploadBuffer(const void * srcData, UINT64 sizeInBytes);

		void SetFormat(DXGI_FORMAT format);

		void CreateResources(ID3D12Device * device, const D3D12_RESOURCE_DESC & resDesc, const void * srcData, UINT64 sizeInBytes, DXGI_FORMAT format);

		virtual void CreateResources(ID3D12Device * device)  override;

		virtual void ReleaseResources() override;

		virtual void UploadResources(IResourceUploader * uploader)  override;

		virtual void ReleaseUploadResources() override;
	};

}
