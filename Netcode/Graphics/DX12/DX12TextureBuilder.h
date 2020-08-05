#pragma once

#include <Netcode/HandleTypes.h>
#include <DirectXTex.h>

namespace Netcode::Graphics::DX12 {

	class TextureBuilderImpl : public TextureBuilder {
		DirectX::ScratchImage scratchImage;
		DirectX::TexMetadata metaData;
	public:

		virtual void LoadTexture2D(const std::wstring & mediaPath) override;

		virtual void LoadTexture3D(const std::wstring & mediaPath) override;

		virtual void LoadTextureCube(const std::wstring & mediaPath) override;

		virtual void LoadTexture2D(ArrayView<uint8_t> data) override;

		virtual void LoadTexture3D(ArrayView<uint8_t> data) override;

		virtual void LoadTextureCube(ArrayView<uint8_t> data) override;

		virtual uint16_t GetCurrentMipLevelCount() override;

		virtual void GenerateMipLevels(uint16_t mipLevelCount) override;

		virtual Ref<Texture> Build() override;
	};

}
