#include "DX12CommittedTexture2D.h"
#include "Utility.h"

namespace Egg::Graphics::DX12::Resource {

	CommittedTexture2D::CommittedTexture2D() noexcept : resource{ nullptr }, resourceDesc{}, isUploaded{ false }{ }

	CommittedTexture2D & CommittedTexture2D::operator=(CommittedTexture2D t) noexcept {
		std::swap(resource, t.resource);
		resourceDesc = t.resourceDesc;
		return *this;
	}
	
	void CommittedTexture2D::SetDesc(const D3D12_RESOURCE_DESC & resDesc) noexcept {
		resourceDesc = resDesc;
	}

	void CommittedTexture2D::CreateResources(ID3D12Device * device) {
		DX_API("failed to create committed resource for texture file")
			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(resource.GetAddressOf()));
	}

	void CommittedTexture2D::CreateResources(ID3D12Device * device, const D3D12_RESOURCE_DESC & resDesc, DirectX::ScratchImage && sImage) {
		SetDesc(resDesc);
		CreateResources(device);
		scratchImage = std::move(sImage);
	}

	void CommittedTexture2D::ReleaseResources() {
		resource.Reset();
	}

	void CommittedTexture2D::UploadResources(IResourceUploader* uploader) {
		uploader->Upload(resourceDesc, resource.Get(), scratchImage.GetPixels(), scratchImage.GetPixelsSize());
		uploader->Transition(resource.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
	}

	void CommittedTexture2D::ReleaseUploadResources() {
		scratchImage.Release();
	}

	const D3D12_RESOURCE_DESC & CommittedTexture2D::GetDesc() const noexcept {
		return resourceDesc;
	}

	ID3D12Resource * CommittedTexture2D::GetResource() const {
		return resource.Get();
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC CommittedTexture2D::GetSRV() const {
		D3D12_SHADER_RESOURCE_VIEW_DESC srvd;
		ZeroMemory(&srvd, sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC));
		srvd.Format = resourceDesc.Format;
		srvd.Texture2D.MipLevels = resourceDesc.MipLevels;
		srvd.Texture2D.MostDetailedMip = 0;
		srvd.Texture2D.ResourceMinLODClamp = 0.0f;
		srvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		return srvd;
	}


}

