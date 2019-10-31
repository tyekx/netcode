#include "CommittedTexture2D.h"
#include <DirectXTex/DirectXTex.h>

namespace Egg::Graphics::Resource::Committed {



	Texture2D::Texture2D() noexcept : resource{ nullptr }, uploadResource{ nullptr }, resourceDesc{} { }

	Texture2D & Texture2D::operator=(Texture2D t) noexcept {
		std::swap(resource, t.resource);
		std::swap(uploadResource, t.uploadResource);
		resourceDesc = t.resourceDesc;
		return *this;
	}
	
	void Texture2D::SetDesc(const D3D12_RESOURCE_DESC & resDesc) noexcept {
		resourceDesc = resDesc;
	}

	void Texture2D::CopyToUploadBuffer(const void * data, UINT64 sizeInBytes) {
		CD3DX12_RANGE readRange{ 0,0 };
		void * mappedPtr;

		DX_API("Failed to map upload resource")
			uploadResource->Map(0, &readRange, &mappedPtr);

		memcpy(mappedPtr, data, sizeInBytes);

		uploadResource->Unmap(0, nullptr);
	}

	void Texture2D::CreateResources(ID3D12Device * device) {
		DX_API("failed to create committed resource for texture file")
			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(resource.GetAddressOf()));

		UINT64 copyableSize;
		device->GetCopyableFootprints(&resourceDesc, 0, 1, 0, nullptr, nullptr, nullptr, &copyableSize);

		DX_API("failed to create committed resource for texture file (upload buffer)")
			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(copyableSize),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(uploadResource.GetAddressOf()));
	}

	void Texture2D::CreateResources(ID3D12Device * device, const D3D12_RESOURCE_DESC & resDesc, const void * data, UINT64 sizeInBytes) {
		SetDesc(resDesc);
		CreateResources(device);
		CopyToUploadBuffer(data, sizeInBytes);
	}

	void Texture2D::ReleaseResources() {
		resource.Reset();
	}

	void Texture2D::UploadResources(ID3D12GraphicsCommandList * copyCommandList) {
		CD3DX12_TEXTURE_COPY_LOCATION dst{ resource.Get(), 0 };
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT psf;
		psf.Offset = 0;
		psf.Footprint.Depth = 1;
		psf.Footprint.Height = (UINT32)resourceDesc.Height;
		psf.Footprint.Width = (UINT32)resourceDesc.Width;
		psf.Footprint.RowPitch = (UINT32)((DirectX::BitsPerPixel(resourceDesc.Format) / 8U) * resourceDesc.Width);
		psf.Footprint.Format = resourceDesc.Format;
		CD3DX12_TEXTURE_COPY_LOCATION src{ uploadResource.Get(), psf };
		copyCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		copyCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
	}

	void Texture2D::ReleaseUploadResources() {
		uploadResource.Reset();
	}

	const D3D12_RESOURCE_DESC & Texture2D::GetDesc() const noexcept {
		return resourceDesc;
	}

	void Texture2D::CreateShaderResourceView(ID3D12Device * device, D3D12_CPU_DESCRIPTOR_HANDLE dHandle) {
		D3D12_SHADER_RESOURCE_VIEW_DESC srvd;
		ZeroMemory(&srvd, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
		srvd.Format = resourceDesc.Format;
		srvd.Texture2D.MipLevels = resourceDesc.MipLevels;
		srvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		device->CreateShaderResourceView(resource.Get(), &srvd, dHandle);
	}


}

