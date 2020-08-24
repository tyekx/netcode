#include "DX12TextureLibrary.h"
#include <Netcode/Modules.h>
#include <Netcode/Graphics/UploadBatch.h>
#include <Netcode/Graphics/ResourceEnums.h>
#include <DirectXTex.h>
#include "DX12Texture.h"

namespace Netcode::Graphics::DX12 {

	TextureLibrary::TextureLibrary(Memory::ObjectAllocator allocator, Module::IGraphicsModule * graphics) :
		objectAllocator{ allocator }, entries{ allocator }, graphics{ graphics }
	{
	}

	void TextureLibrary::ClearCache() {
		entries.clear();
	}

	Ref<GpuResource> TextureLibrary::GetTexture(const URI::Texture & uri, uint16_t mipLevels)
	{
		if(uri.Empty()) {
			return nullptr;
		}

		for(const auto & entry : entries) {
			if(entry.uri.GetFullPath() == uri.GetFullPath() &&
				entry.mipLevels == mipLevels) {
				return entry.resource;
			}
		}

		return nullptr;
	}

	Ref<GpuResource> TextureLibrary::CreateTexture2D(const URI::Texture & uri, ResourceState stateAfterUpload, DirectX::ScratchImage scratchImage, DirectX::TexMetadata metaData)
	{
		Ref<GpuResource> resource = graphics->resources->CreateTexture2D(scratchImage.GetImage(0, 0, 0), metaData.mipLevels);

		auto texture = std::make_shared<TextureImpl>(std::move(scratchImage));

		auto uploadBatch = graphics->resources->CreateUploadBatch();
		uploadBatch->Upload(resource, texture);
		uploadBatch->Barrier(resource, ResourceState::COPY_DEST, stateAfterUpload);
		graphics->frame->SyncUpload(uploadBatch);

		if(uri.Empty()) {
			return resource;
		}

		TextureEntry entry;
		entry.resource = resource;
		entry.uri = uri;
		entry.mipLevels = metaData.mipLevels;
		entries.emplace_back(std::move(entry));

		return resource;
	}

	Ref<GpuResource> TextureLibrary::CreateTextureCube(const URI::Texture & uri, ResourceState stateAfterUpload, DirectX::ScratchImage scratchImage, DirectX::TexMetadata metaData)
	{
		Ref<GpuResource> resource = graphics->resources->CreateTextureCube(static_cast<uint32_t>(metaData.width), static_cast<uint32_t>(metaData.height), static_cast<uint16_t>(metaData.mipLevels), metaData.format, ResourceType::PERMANENT_DEFAULT, ResourceState::COPY_DEST, ResourceFlags::NONE);

		auto texture = std::make_shared<TextureImpl>(std::move(scratchImage));

		auto uploadBatch = graphics->resources->CreateUploadBatch();
		uploadBatch->Upload(resource, texture);
		uploadBatch->Barrier(resource, ResourceState::COPY_DEST, stateAfterUpload);
		graphics->frame->SyncUpload(uploadBatch);

		if(uri.Empty()) {
			return resource;
		}

		TextureEntry entry;
		entry.resource = resource;
		entry.uri = uri;
		entry.mipLevels = metaData.mipLevels;
		entries.emplace_back(std::move(entry));

		return resource;
	}



}
