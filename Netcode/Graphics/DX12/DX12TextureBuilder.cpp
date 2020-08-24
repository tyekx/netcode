#include "DX12TextureBuilder.h"
#include "DX12Common.h"
#include "DX12Texture.h"
#include <Netcode/IO/Path.h>
#include <Netcode/IO/File.h>
#include <NetcodeFoundation/ArrayView.hpp>
#include <NetcodeFoundation/Exceptions.h>
#include "DX12TextureLibrary.h"

namespace Netcode::Graphics::DX12 {
	void TextureBuilderImpl::SetDefaults()
	{
		uri = URI::Texture{};
		scratchImage.Release();
		metaData = DirectX::TexMetadata{};
		stateAfterUpload = ResourceState::ANY_READ;
		dimension = TexDim::NONE;
		mipLevels = 1;
	}

	void TextureBuilderImpl::LoadDDSFromMemory(ArrayView<uint8_t> data)
	{
		DX_API("Failed to load DDS image from memory")
			DirectX::LoadFromDDSMemory(data.Data(), data.Size(), DirectX::DDS_FLAGS_NONE, &metaData, scratchImage);
	}

	void TextureBuilderImpl::LoadWICFromMemory(ArrayView<uint8_t> data)
	{
		DX_API("Failed to load image from memory")
			DirectX::LoadFromWICMemory(data.Data(), data.Size(), DirectX::WIC_FLAGS_NONE, &metaData, scratchImage);
	}

	TextureBuilderImpl::TextureBuilderImpl(Ref<TextureLibrary> texLib) : textureLibrary{ std::move(texLib) },
		uri{},
		scratchImage{},
		metaData{},
		stateAfterUpload{ ResourceState::ANY_READ },
		dimension{ TexDim::NONE },
		mipLevels{ 1 }
	{

	}

	void TextureBuilderImpl::LoadTexture2D(const URI::Texture & mediaPath) {
		uri = mediaPath;
		dimension = TexDim::TEXTURE_2D;
	}
	
	void TextureBuilderImpl::LoadTexture3D(const URI::Texture & mediaPath) {
		uri = mediaPath;
		NotImplementedAssertion("Loading Texture3D is not supported yet");
	}
	
	void TextureBuilderImpl::LoadTextureCube(const URI::Texture & mediaPath) {
		uri = mediaPath;
		dimension = TexDim::TEXTURE_CUBE;
	}

	void TextureBuilderImpl::LoadTexture2D(Netcode::ArrayView<uint8_t> data)
	{
		dimension = TexDim::TEXTURE_2D;
		uri = URI::Texture{};
		LoadWICFromMemory(data);
	}

	void TextureBuilderImpl::LoadTexture3D(Netcode::ArrayView<uint8_t> data)
	{
		uri = URI::Texture{};
		NotImplementedAssertion("Loading Texture3D is not supported yet");
	}

	void TextureBuilderImpl::LoadTextureCube(Netcode::ArrayView<uint8_t> data)
	{
		uri = URI::Texture{};
		dimension = TexDim::TEXTURE_CUBE;
		LoadDDSFromMemory(data);
	}
	
	void TextureBuilderImpl::SetMipLevels(uint16_t mipLevelCount) {
		mipLevels = mipLevelCount;
	}

	void TextureBuilderImpl::SetStateAfterUpload(ResourceState state)
	{
		stateAfterUpload = state;
	}
	
	Ref<GpuResource> TextureBuilderImpl::Build() {
		Ref<GpuResource> existingResource = textureLibrary->GetTexture(uri, mipLevels);
		if(existingResource != nullptr) {
			return existingResource;
		}

		if(!uri.Empty()) {
			IO::File file{ uri.GetTexturePath() };
			IO::FileReader<IO::File> reader{ file, IO::FileOpenMode::READ_BINARY };
			size_t size = reader->GetSize();
			std::unique_ptr<uint8_t[]> tmpBuffer = std::make_unique<uint8_t[]>(size);
			MutableArrayView<uint8_t> buffer{ tmpBuffer.get(), size };
			reader->Read(buffer);
			reader->Close();

			if(_wcsnicmp(file.GetExtension().data(), L"dds", file.GetExtension().size()) == 0) {
				LoadDDSFromMemory(buffer);
			} else {
				LoadWICFromMemory(buffer);
			}
		}

		if(mipLevels > static_cast<uint16_t>(metaData.mipLevels)) {
			DirectX::ScratchImage outIm;

			DX_API("Failed to generate mip levels")
				DirectX::GenerateMipMaps(scratchImage.GetImages(), scratchImage.GetImageCount(), scratchImage.GetMetadata(), DirectX::TEX_FILTER_BOX, static_cast<size_t>(mipLevels), outIm);

			metaData = outIm.GetMetadata();

			std::swap(outIm, scratchImage);
		}

		Ref<GpuResource> resource;

		switch(dimension) {
			case TexDim::TEXTURE_2D:
				resource = textureLibrary->CreateTexture2D(uri, stateAfterUpload, std::move(scratchImage), metaData);
				break;
			case TexDim::TEXTURE_CUBE:
				resource = textureLibrary->CreateTextureCube(uri, stateAfterUpload, std::move(scratchImage), metaData);
				break;
			default: break;
		}

		SetDefaults();

		return resource;
	}

}
