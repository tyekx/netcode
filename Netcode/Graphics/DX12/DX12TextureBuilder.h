#pragma once

#include "DX12Texture.h"

namespace Netcode::Graphics::DX12 {

	class TextureBuilder : public Netcode::TextureBuilder {
		DirectX::ScratchImage scratchImage;
		DirectX::TexMetadata metaData;
	public:

		virtual void LoadTexture2D(const std::wstring & mediaPath) override;

		virtual void LoadTexture3D(const std::wstring & mediaPath) override;

		virtual void LoadTextureCube(const std::wstring & mediaPath) override;

		virtual void LoadTexture2D(Netcode::ArrayView<uint8_t> data) override;

		virtual void LoadTexture3D(Netcode::ArrayView<uint8_t> data) override;

		virtual void LoadTextureCube(Netcode::ArrayView<uint8_t> data) override;

		virtual uint16_t GetCurrentMipLevelCount() override;

		virtual void GenerateMipLevels(uint16_t mipLevelCount) override;

		virtual Ref<Netcode::Texture> Build() override;
	};

}
