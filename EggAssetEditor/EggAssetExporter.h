#pragma once

#include "Model.h"
#include "Manifest.h"

class EggAssetExporter {
	EggAssetExporter() = delete;
	~EggAssetExporter() = delete;
public:
	static void Export(const std::string & path, const Model & model);
};
