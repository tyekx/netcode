#include "DX12TextureBuilder.h"
#include "Path.h"
#include "DX12Common.h"

namespace Egg::Graphics::DX12 {

	void TextureBuilder::LoadTexture2D(const std::wstring & mediaPath) {
		MediaPath media{ mediaPath };

		DX_API("Failed to load image: %S", mediaPath.c_str())
			DirectX::LoadFromWICFile(media.GetAbsolutePath().c_str(), 0, &metaData, scratchImage);
	}
	
	void TextureBuilder::LoadTexture3D(const std::wstring & mediaPath) {
		ASSERT(false, "Loading Texture3D is not supported yet");
	}
	
	void TextureBuilder::LoadTextureCube(const std::wstring & mediaPath) {
		MediaPath media{ mediaPath };

		DX_API("Failed to load image: %S", mediaPath.c_str())
			DirectX::LoadFromDDSFile(media.GetAbsolutePath().c_str(), 0, &metaData, scratchImage);
	}
	
	uint16_t TextureBuilder::GetCurrentMipLevelCount() {
		return static_cast<uint16_t>(metaData.mipLevels);
	}
	
	void TextureBuilder::GenerateMipLevels(uint16_t mipLevelCount) {
		DirectX::ScratchImage outIm;

		DX_API("Failed to generate mip levels")
			DirectX::GenerateMipMaps(scratchImage.GetImages(), scratchImage.GetImageCount(), scratchImage.GetMetadata(), DirectX::TEX_FILTER_BOX, static_cast<size_t>(mipLevelCount), outIm);

		std::swap(outIm, scratchImage);
	}
	
	TextureRef TextureBuilder::Build() {
		return std::make_shared<DX12Texture>(std::move(scratchImage));
	}

}
