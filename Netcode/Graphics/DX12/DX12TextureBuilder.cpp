#include "DX12TextureBuilder.h"
#include "DX12Common.h"

#include "../../IO/Path.h"

#include <NetcodeFoundation/Exceptions.h>

namespace Netcode::Graphics::DX12 {

	void TextureBuilder::LoadTexture2D(const std::wstring & mediaPath) {
		std::wstring cpy{ mediaPath };
		cpy.insert(0, IO::Path::MediaRoot());

		DX_API("Failed to load image: %S", cpy.c_str())
			DirectX::LoadFromWICFile(cpy.c_str(), 0, &metaData, scratchImage);
	}
	
	void TextureBuilder::LoadTexture3D(const std::wstring & mediaPath) {
		NotImplementedAssertion("Loading Texture3D is not supported yet");
	}
	
	void TextureBuilder::LoadTextureCube(const std::wstring & mediaPath) {
		std::wstring cpy{ mediaPath };
		cpy.insert(0, IO::Path::MediaRoot());

		DX_API("Failed to load image: %S", cpy.c_str())
			DirectX::LoadFromDDSFile(cpy.c_str(), 0, &metaData, scratchImage);
	}

	void TextureBuilder::LoadTexture2D(Netcode::ArrayView<uint8_t> data)
	{
		DX_API("Failed to load image from memory")
			DirectX::LoadFromWICMemory(data.Data(), data.Size(), 0, &metaData, scratchImage);
	}

	void TextureBuilder::LoadTexture3D(Netcode::ArrayView<uint8_t> data)
	{
		NotImplementedAssertion("Loading Texture3D is not supported yet");
	}

	void TextureBuilder::LoadTextureCube(Netcode::ArrayView<uint8_t> data)
	{
		DX_API("Failed to load DDS image from memory")
			DirectX::LoadFromDDSMemory(data.Data(), data.Size(), 0, &metaData, scratchImage);
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
	
	Ref<Netcode::Texture> TextureBuilder::Build() {
		return std::make_shared<DX12::Texture>(std::move(scratchImage));
	}

}
