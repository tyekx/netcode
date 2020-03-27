#pragma once

#include "../../HandleTypes.h"
#include <DirectXTex/DirectXTex.h>

namespace Egg::Graphics::DX12 {

	class Texture : public Egg::Texture {
		DirectX::ScratchImage textureData;
	public:

		Texture(DirectX::ScratchImage && tData);

		virtual Egg::Graphics::ResourceDimension GetDimension() const override;

		virtual uint16_t GetMipLevelCount() const override;

		virtual const Image * GetImage(uint16_t mipIndex, uint16_t arrayIndex, uint32_t slice) override;

		virtual const Image * GetImages() override;

		virtual uint16_t GetImageCount() override;
	};

	using DX12Texture = Egg::Graphics::DX12::Texture;
	using DX12TextureRef = std::shared_ptr<Egg::Graphics::DX12::Texture>;

}

