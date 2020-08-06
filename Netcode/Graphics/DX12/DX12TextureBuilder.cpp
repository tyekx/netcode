#include "DX12TextureBuilder.h"
#include "DX12Common.h"
#include "DX12Texture.h"
#include <Netcode/IO/Path.h>
#include <Netcode/IO/File.h>
#include <Netcode/URI/Texture.h>
#include <NetcodeFoundation/ArrayView.hpp>
#include <NetcodeFoundation/Exceptions.h>

namespace Netcode::Graphics::DX12 {

	void TextureBuilderImpl::LoadTexture2D(const URI::Texture & mediaPath) {

		IO::File file{ mediaPath.GetTexturePath() };
		IO::FileReader<IO::File> reader{ file, IO::FileOpenMode::READ_BINARY };
		size_t size = reader->GetSize();
		std::unique_ptr<uint8_t[]> tmpBuffer = std::make_unique<uint8_t[]>(size);
		MutableArrayView<uint8_t> buffer{ tmpBuffer.get(), size };
		reader->Read(buffer);
		reader->Close();

		LoadTexture2D(buffer);
	}
	
	void TextureBuilderImpl::LoadTexture3D(const URI::Texture & mediaPath) {
		NotImplementedAssertion("Loading Texture3D is not supported yet");
	}
	
	void TextureBuilderImpl::LoadTextureCube(const URI::Texture & mediaPath) {
		IO::File file{ mediaPath.GetTexturePath() };
		IO::FileReader<IO::File> reader{ file, IO::FileOpenMode::READ_BINARY };
		size_t size = reader->GetSize();
		std::unique_ptr<uint8_t[]> tmpBuffer = std::make_unique<uint8_t[]>(size);
		MutableArrayView<uint8_t> buffer{ tmpBuffer.get(), size };
		reader->Read(buffer);
		reader->Close();

		LoadTextureCube(buffer);
	}

	void TextureBuilderImpl::LoadTexture2D(Netcode::ArrayView<uint8_t> data)
	{
		DX_API("Failed to load image from memory")
			DirectX::LoadFromWICMemory(data.Data(), data.Size(), 0, &metaData, scratchImage);
	}

	void TextureBuilderImpl::LoadTexture3D(Netcode::ArrayView<uint8_t> data)
	{
		NotImplementedAssertion("Loading Texture3D is not supported yet");
	}

	void TextureBuilderImpl::LoadTextureCube(Netcode::ArrayView<uint8_t> data)
	{
		DX_API("Failed to load DDS image from memory")
			DirectX::LoadFromDDSMemory(data.Data(), data.Size(), 0, &metaData, scratchImage);
	}
	
	uint16_t TextureBuilderImpl::GetCurrentMipLevelCount() {
		return static_cast<uint16_t>(metaData.mipLevels);
	}
	
	void TextureBuilderImpl::GenerateMipLevels(uint16_t mipLevelCount) {
		DirectX::ScratchImage outIm;

		DX_API("Failed to generate mip levels")
			DirectX::GenerateMipMaps(scratchImage.GetImages(), scratchImage.GetImageCount(), scratchImage.GetMetadata(), DirectX::TEX_FILTER_BOX, static_cast<size_t>(mipLevelCount), outIm);

		std::swap(outIm, scratchImage);
	}
	
	Ref<Texture> TextureBuilderImpl::Build() {
		return std::make_shared<TextureImpl>(std::move(scratchImage));
	}

}
