#pragma once

#include "Model.h"

class EggAssetExporter {
	EggAssetExporter() = delete;
	~EggAssetExporter() = delete;
public:
	static std::tuple<std::unique_ptr<uint8_t[]>, size_t> Export(const Model & model);
};
