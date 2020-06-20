#pragma once

#include <NetcodeAssetLib/Model.h>
#include <map>
#include <string>
#include <Netcode/Utility.h>
#include <NetcodeAssetLib/Importer.h>

#include <Netcode/IO/File.h>
#include <Netcode/IO/Path.h>

__declspec(align(16))
class AssetManager {

	std::map<std::wstring, std::unique_ptr<Netcode::Asset::Model>> storage;

	Netcode::Asset::Model * ImportFromFile(std::wstring str) {
		std::unique_ptr<Netcode::Asset::Model> model = std::make_unique<Netcode::Asset::Model>();

		Netcode::Asset::Model * rawPtr = model.get();

		Netcode::IO::File modelFile{ Netcode::IO::Path::MediaRoot(), str };
		Netcode::IO::FileReader<Netcode::IO::File> reader{ modelFile, Netcode::IO::FileOpenMode::READ };
		size_t n = reader->GetSize();
		std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(n);

		Netcode::MutableArrayView<uint8_t> mutableView{ buffer.get(), n };

		reader->Read(mutableView);

		Netcode::Asset::ImportModel(mutableView, *rawPtr);

		storage.emplace(std::move(str), std::move(model));

		return rawPtr;
	}

public:

	Netcode::Asset::Model * Import(std::wstring relativeMediaPath) {
		const auto it = storage.find(relativeMediaPath);

		if(it != std::end(storage)) {
			return it->second.get();
		}

		return ImportFromFile(std::move(relativeMediaPath));
	}

	void Clear() {
		storage.clear();
	}

};
