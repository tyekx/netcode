#pragma once

#include <NetcodeAssetLib/Model.h>
#include <map>
#include <string>
#include <Netcode/Utility.h>
#include <Netcode/Path.h>
#include <NetcodeAssetLib/Importer.h>

__declspec(align(16))
class AssetManager {

	std::map<std::string, std::unique_ptr<Netcode::Asset::Model>> storage;

	Netcode::Asset::Model * ImportFromFile(std::string str) {
		std::unique_ptr<Netcode::Asset::Model> model = std::make_unique<Netcode::Asset::Model>();

		Netcode::Asset::Model * rawPtr = model.get();

		Netcode::Asset::ImportModel(str.c_str(), *rawPtr);

		storage.emplace(std::move(str), std::move(model));

		return rawPtr;
	}

public:

	Netcode::Asset::Model * Import(const Netcode::MediaPath & path) {
		std::string key = Netcode::Utility::ToNarrowString(path.GetAbsolutePath());

		const auto it = storage.find(key);

		if(it != std::end(storage)) {
			return it->second.get();
		}

		return ImportFromFile(std::move(key));
	}

	void Clear() {
		storage.clear();
	}

};
