#pragma once

#include "Model.h"
#include "Manifest.h"

class EggAssetExporter {
	EggAssetExporter() = delete;
	~EggAssetExporter() = delete;
public:
	static std::vector<uint8_t> Export(const Model & model);
};
