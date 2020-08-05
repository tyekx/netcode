#pragma once

#include <Netcode/HandleTypes.h>
#include <DirectXTex.h>

namespace Netcode::Graphics::DX12 {

	class TextureImpl : public Netcode::Texture {
		DirectX::ScratchImage textureData;
	public:

		TextureImpl(DirectX::ScratchImage && tData);

		virtual Netcode::Graphics::ResourceDimension GetDimension() const override;

		virtual uint16_t GetMipLevelCount() const override;

		virtual const Image * GetImage(uint16_t mipIndex, uint16_t arrayIndex, uint32_t slice) override;

		virtual const Image * GetImages() override;

		virtual uint16_t GetImageCount() override;
	};

}

