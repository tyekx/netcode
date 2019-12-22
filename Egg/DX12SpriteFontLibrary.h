#pragma once

#include "DX12Common.h"
#include "DX12SpriteFont.h"
#include "HandleTypes.h"
#include "Path.h"
#include "DX12TextureLibrary.h"

namespace Egg::Graphics::DX12 {

	class SpriteFontLibrary {

		struct Item {
			std::unique_ptr<SpriteFont> font;
			std::wstring name;

			Item() = default;
			Item(std::unique_ptr<SpriteFont> && f,const std::wstring & n) : font{ std::move(f) }, name{ n } { }

		};

		std::vector<Item> storage;
		ID3D12Device * device;
		TextureLibrary * textureLibrary;

		std::unique_ptr<SpriteFont> Load(const std::wstring & fontPath, Resource::IResourceUploader * upload) {
			D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorDest;
			D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptor;
			textureLibrary->AllocateTextures(gpuDescriptor, cpuDescriptorDest, 1);
			return std::make_unique<SpriteFont>(device, upload, fontPath.c_str(), cpuDescriptorDest, gpuDescriptor);
		}

	public:

		void CreateResources(ID3D12Device * dev, TextureLibrary * texLib) {
			device = dev;
			textureLibrary = texLib;
		}

		SpriteFont * Get(HFONT font) {
			return storage[font].font.get();
		}

		HFONT LoadFont(const std::wstring & fontName, Resource::IResourceUploader * upload) {

			Egg::MediaPath mediaPath{ fontName };
			std::wstring fontPath = mediaPath.GetAbsolutePath();

			HFONT idx = 0;
			for(const Item & i : storage) {
				if(i.name == fontName) {
					return idx;
				}
				idx += 1;
			}

			std::unique_ptr<SpriteFont> font = Load(fontPath, upload);

			storage.emplace_back(std::move(font), fontName);

			return idx;
		}


	};

}
