#pragma once

#include "DX12Texture.h"

namespace Egg::Graphics::DX12 {

	class TextureBuilder : public Egg::TextureBuilder {
		DirectX::ScratchImage scratchImage;
		DirectX::TexMetadata metaData;
	public:
		virtual void LoadTexture2D(const std::wstring & mediaPath) override;

		virtual void LoadTexture3D(const std::wstring & mediaPath) override;

		virtual void LoadTextureCube(const std::wstring & mediaPath) override;

		virtual uint16_t GetCurrentMipLevelCount() override;

		virtual void GenerateMipLevels(uint16_t mipLevelCount) override;

		virtual TextureRef Build() override;
	};

	using DX12TextureBuilder = Egg::Graphics::DX12::TextureBuilder;
	using DX12TextureBuilderRef = std::shared_ptr<DX12TextureBuilder>;

}
