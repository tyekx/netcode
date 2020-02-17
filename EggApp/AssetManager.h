#pragma once

#include <EggAssetLib/Model.h>
#include <map>
#include <string>
#include <Egg/Utility.h>
#include <Egg/Path.h>
#include <EggAssetLib/Importer.h>

class AssetManager {

	std::map<std::string, std::unique_ptr<Egg::Asset::Model>> storage;

	Egg::Asset::Model * ImportFromFile(std::string str) {
		std::unique_ptr<Egg::Asset::Model> model = std::make_unique<Egg::Asset::Model>();

		Egg::Asset::Model * rawPtr = model.get();

		Egg::Asset::ImportModel(str.c_str(), *rawPtr);

		return rawPtr;
	}

public:

	Egg::Asset::Model * Import(const Egg::MediaPath & path) {
		std::string key = Egg::Utility::ToNarrowString(path.GetAbsolutePath());

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
