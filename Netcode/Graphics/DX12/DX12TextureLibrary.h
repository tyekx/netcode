#pragma once

#include <Netcode/HandleDecl.h>
#include <Netcode/URI/Texture.h>
#include "DX12Common.h"

namespace DirectX {
	class ScratchImage;
	struct TexMetadata;
}

namespace Netcode::Module {

	class IGraphicsModule;

}

namespace Netcode::Graphics::DX12 {

	class TextureImpl;

	class TextureLibrary {
	public:
		Memory::ObjectAllocator objectAllocator;

		struct TextureEntry {
			Ref<GpuResource> resource;
			URI::Texture uri;
			uint16_t mipLevels;
		};

		BuilderContainer<TextureEntry> entries;
		Module::IGraphicsModule * graphics;

		TextureLibrary(Memory::ObjectAllocator allocator, Module::IGraphicsModule* graphics);

		void ClearCache();

		Ref<GpuResource> GetTexture(const URI::Texture & uri, uint16_t mipLevels);

		Ref<GpuResource> CreateTexture2D(const URI::Texture & uri, ResourceState stateAfterUpload, DirectX::ScratchImage scratchImage, DirectX::TexMetadata metaData);
		Ref<GpuResource> CreateTextureCube(const URI::Texture & uri, ResourceState stateAfterUpload, DirectX::ScratchImage scratchImage, DirectX::TexMetadata metaData);
	};

}
