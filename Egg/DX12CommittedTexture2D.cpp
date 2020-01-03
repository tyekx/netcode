#include "DX12CommittedTexture2D.h"
#include "Utility.h"

namespace Egg::Graphics::DX12::Resource {

	CommittedTexture2D::CommittedTexture2D() noexcept : resource{ nullptr }, resourceDesc{}, isUploaded{ false }{ }

	CommittedTexture2D & CommittedTexture2D::operator=(CommittedTexture2D t) noexcept {
		std::swap(resource, t.resource);
		resourceDesc = t.resourceDesc;
		return *this;
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

	void CommittedTexture2D::CreateResources(ID3D12Device * device, DirectX::ScratchImage && sImage) {
		scratchImage = std::move(sImage);
		
		DirectX::TexMetadata meta = scratchImage.GetMetadata();

		resourceDesc.Alignment = 0;
		resourceDesc.DepthOrArraySize = static_cast<UINT16>(meta.arraySize);
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resourceDesc.Format = meta.format;
		resourceDesc.Height = meta.height;
		resourceDesc.Width = meta.width;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.MipLevels = static_cast<UINT16>(meta.mipLevels);
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;

		CreateResources(device);
	}

	void CommittedTexture2D::ReleaseResources() {
		resource.Reset();
	}

	void CommittedTexture2D::UploadResources(IResourceUploader* uploader) {
		uploader->Upload(resource.Get(), scratchImage.GetImages(), scratchImage.GetImageCount());
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
		D3D12_SHADER_RESOURCE_VIEW_DESC srvd = {};
		srvd.Format = resourceDesc.Format;
		srvd.Texture2D.MipLevels = resourceDesc.MipLevels;
		srvd.Texture2D.MostDetailedMip = 0;
		srvd.Texture2D.ResourceMinLODClamp = 0.0f;
		srvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		return srvd;
	}


}

