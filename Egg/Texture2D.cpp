#include "Texture2D.h"
#include <DirectXTex/DirectXTex.h>

namespace Egg {


	Texture2D::Texture2D() : resource{ nullptr }, uploadResource{ nullptr }, resourceDesc{} { }

	// copy-and-swap idiom (c++11)
	Texture2D & Texture2D::operator=(Texture2D t) {
		std::swap(resource, t.resource);
		std::swap(uploadResource, t.uploadResource);
		resourceDesc = t.resourceDesc;
		return *this;
	}

	Texture2D::Texture2D(com_ptr<ID3D12Resource> && resource, com_ptr<ID3D12Resource> && uploadResource, D3D12_RESOURCE_DESC resourceDesc) :
		resource{ std::move(resource) },
		uploadResource{ std::move(uploadResource) },
		resourceDesc{ resourceDesc } {

	}

	void Texture2D::UploadResource(ID3D12GraphicsCommandList * commandList) {
		CD3DX12_TEXTURE_COPY_LOCATION dst{ resource.Get(), 0 };
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT psf;
		psf.Offset = 0;
		psf.Footprint.Depth = 1;
		psf.Footprint.Height = resourceDesc.Height;
		psf.Footprint.Width = resourceDesc.Width;
		psf.Footprint.RowPitch = (DirectX::BitsPerPixel(resourceDesc.Format) / 8U) * resourceDesc.Width;
		psf.Footprint.Format = resourceDesc.Format;
		CD3DX12_TEXTURE_COPY_LOCATION src{ uploadResource.Get(), psf };
		commandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
	}

	void Texture2D::ReleaseUploadResources() {
		uploadResource.Reset();
	}


}