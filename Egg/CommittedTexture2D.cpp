#include "CommittedTexture2D.h"
#include <DirectXTex/DirectXTex.h>
#include "Utility.h"

namespace Egg::Graphics::Resource::Committed {



	Texture2D::Texture2D() noexcept : resource { nullptr }, uploadResource{ nullptr }, resourceDesc{} { }

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
		device->GetCopyableFootprints(&resourceDesc, 0, resourceDesc.MipLevels, 0, nullptr, nullptr, nullptr, &copyableSize);

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
		if(uploadResource == nullptr) {
			return;
		}
		UINT offset = 0;
		UINT bytesPerPixel = (DirectX::BitsPerPixel(resourceDesc.Format) / 8U);
		for(UINT i = 0; i < resourceDesc.MipLevels; ++i) {
			CD3DX12_TEXTURE_COPY_LOCATION dst{ resource.Get(), i };

			UINT width = ((UINT)resourceDesc.Width) >> i;
			UINT height = ((UINT)resourceDesc.Height) >> i; 

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT psf;
			psf.Offset = offset;
			psf.Footprint.Depth = 1;
			psf.Footprint.Height = height;
			psf.Footprint.Width = width;
			psf.Footprint.RowPitch = psf.Footprint.Width * bytesPerPixel;
			psf.Footprint.Format = resourceDesc.Format;
			offset += height * width * bytesPerPixel;
			CD3DX12_TEXTURE_COPY_LOCATION src{ uploadResource.Get(), psf };
			copyCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		}
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
		srvd.Texture2D.MostDetailedMip = 0;
		srvd.Texture2D.ResourceMinLODClamp = 0.0f;
		srvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		device->CreateShaderResourceView(resource.Get(), &srvd, dHandle);
	}


}

