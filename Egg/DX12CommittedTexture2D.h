#pragma once

#include "DX12Resource.h"
#include <DirectXTex/DirectXTex.h>

namespace Egg::Graphics::DX12::Resource {


	/*
	To initialize an instance of Texture2D call:
	- Default constructor
	- CreateResources(3 arguments)
	*/
	class CommittedTexture2D : public ITexture {
		com_ptr<ID3D12Resource> resource;
		D3D12_RESOURCE_DESC resourceDesc;
		DirectX::ScratchImage scratchImage;
		bool isUploaded;
	public:

		CommittedTexture2D() noexcept;

		CommittedTexture2D & operator=(CommittedTexture2D t) noexcept;

		void CreateResources(ID3D12Device * device, DirectX::ScratchImage && sImage);

		virtual void CreateResources(ID3D12Device * device) override;

		virtual void ReleaseResources() override;

		virtual void UploadResources(IResourceUploader * uploader) override;

		virtual void ReleaseUploadResources() override;

		virtual const D3D12_RESOURCE_DESC & GetDesc() const noexcept;

		D3D12_SHADER_RESOURCE_VIEW_DESC GetSRV() const override;

		virtual ID3D12Resource * GetResource() const override;
	};

}
