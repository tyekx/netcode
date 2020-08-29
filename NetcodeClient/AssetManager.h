#pragma once

#include <Netcode/HandleTypes.h>
#include <Netcode/Modules.h>
#include <NetcodeAssetLib/Model.h>
#include <map>
#include <string>
#include <Netcode/Utility.h>
#include <NetcodeAssetLib/Importer.h>

#include <Netcode/IO/File.h>
#include <Netcode/IO/Path.h>

#include <Netcode/Graphics/UploadBatch.h>
#include <Netcode/Graphics/ResourceEnums.h>
#include <Netcode/Graphics/Material.h>

#include <Netcode/URI/Texture.h>
#include <Netcode/URI/Model.h>

class AssetManager {

	Netcode::Module::IGraphicsModule * graphics;

	struct StorageItem {
		Netcode::URI::Model uri;
		std::unique_ptr<Netcode::Asset::Model> model;
	};

	std::vector<StorageItem> storage;

	Netcode::Asset::Model * ImportFromFile(const Netcode::URI::Model & uri) {
		std::unique_ptr<Netcode::Asset::Model> model = std::make_unique<Netcode::Asset::Model>();

		Netcode::Asset::Model * rawPtr = model.get();

		Netcode::IO::File modelFile{ uri.GetModelPath() };
		Netcode::IO::FileReader<Netcode::IO::File> reader{ modelFile, Netcode::IO::FileOpenMode::READ };
		size_t n = reader->GetSize();
		std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(n);

		Netcode::MutableArrayView<uint8_t> mutableView{ buffer.get(), n };

		reader->Read(mutableView);

		Netcode::Asset::ImportModel(mutableView, *rawPtr);

		StorageItem si;
		si.model = std::move(model);
		si.uri = uri;

		storage.emplace_back(std::move(si));

		return rawPtr;
	}

public:

	AssetManager(Netcode::Module::IGraphicsModule * g) : graphics{ g }, storage{} {

	}

	Ref<Netcode::SpriteFont> ImportFont(const std::wstring & relativeMediaPath) {
		Ref<Netcode::SpriteFontBuilder> spriteFontBuilder = graphics->CreateSpriteFontBuilder();
		spriteFontBuilder->LoadFont(relativeMediaPath);
		return spriteFontBuilder->Build();
	}

	Ref<Netcode::GpuResource> ImportTexture2D(const std::wstring & relativeMediaPath) {
		Ref<Netcode::TextureBuilder> builder = graphics->CreateTextureBuilder();

		builder->LoadTexture2D(Netcode::URI::Texture{ relativeMediaPath });

		return builder->Build();
	}

	Ref<Netcode::ResourceViews> CreateTextureRV(Ref<Netcode::GpuResource> resourceRef) {
		Ref<Netcode::ResourceViews> rv = graphics->resources->CreateShaderResourceViews(1);
		rv->CreateSRV(0, resourceRef.get());
		return rv;
	}

	Netcode::Asset::Model * Import(const Netcode::URI::Model & uri) {
		auto it = std::find_if(std::cbegin(storage), std::cend(storage), [&uri](const StorageItem & si) -> bool {
			return si.uri.GetFullPath() == uri.GetFullPath();
		});

		if(it != std::cend(storage)) {
			return it->model.get();
		}

		auto *model = ImportFromFile(uri);

		return model;
	}

	void Clear() {
		storage.clear();
	}

};
