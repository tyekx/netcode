#pragma once

#include <Netcode/HandleTypes.h>
#include <Netcode/URI/Texture.h>
#include <Netcode/Graphics/ResourceEnums.h>
#include <DirectXTex.h>

namespace Netcode::Graphics::DX12 {

	class TextureLibrary;

	enum class TexDim : uint16_t {
		NONE, TEXTURE_2D, TEXTURE_CUBE
	};

	class TextureBuilderImpl : public TextureBuilder {
		Ref<TextureLibrary> textureLibrary;
		URI::Texture uri;
		DirectX::ScratchImage scratchImage;
		DirectX::TexMetadata metaData;
		ResourceState stateAfterUpload;
		TexDim dimension;
		uint16_t mipLevels;

		void SetDefaults();

		void LoadDDSFromMemory(ArrayView<uint8_t> data);
		void LoadWICFromMemory(ArrayView<uint8_t> data);

	public:

		TextureBuilderImpl(Ref<TextureLibrary> texLib);

		virtual void LoadTexture2D(const URI::Texture & mediaPath) override;

		virtual void LoadTexture3D(const URI::Texture & mediaPath) override;

		virtual void LoadTextureCube(const URI::Texture & mediaPath) override;

		virtual void LoadTexture2D(ArrayView<uint8_t> data) override;

		virtual void LoadTexture3D(ArrayView<uint8_t> data) override;

		virtual void LoadTextureCube(ArrayView<uint8_t> data) override;

		virtual void SetMipLevels(uint16_t mipLevels) override;

		virtual void SetStateAfterUpload(ResourceState state) override;

		virtual Ref<GpuResource> Build() override;
	};

}
