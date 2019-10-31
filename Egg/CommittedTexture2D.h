#pragma once

#include "Resource.h"

namespace Egg::Graphics::Resource::Committed {


	/*
	To initialize an instance of Texture2D call:
	- Default constructor
	- CreateResources(4 arguments)
	*/
	class Texture2D : public ITexture {
		com_ptr<ID3D12Resource> uploadResource;
		com_ptr<ID3D12Resource> resource;
		D3D12_RESOURCE_DESC resourceDesc;
	public:

		Texture2D() noexcept;

		Texture2D & operator=(Texture2D t) noexcept;

		void SetDesc(const D3D12_RESOURCE_DESC & resDesc) noexcept;

		void CopyToUploadBuffer(const void * data, UINT64 sizeInBytes);

		void CreateResources(ID3D12Device * device, const D3D12_RESOURCE_DESC & resDesc, const void * data, UINT64 sizeInBytes);

		virtual void CreateResources(ID3D12Device * device) override;

		virtual void ReleaseResources() override;

		virtual void UploadResources(ID3D12GraphicsCommandList * copyCommandList) override;

		virtual void ReleaseUploadResources() override;

		virtual const D3D12_RESOURCE_DESC & GetDesc() const noexcept;

		virtual void CreateShaderResourceView(ID3D12Device * device, D3D12_CPU_DESCRIPTOR_HANDLE dHandle) override;
	};

}
