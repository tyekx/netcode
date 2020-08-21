#pragma once

#include "IntermediateModel.h"

namespace Netcode {

	class AssetExporter {
		AssetExporter() = delete;
		~AssetExporter() = delete;
	public:
		static std::tuple<std::unique_ptr<uint8_t[]>, size_t> Export(const Intermediate::Model & model);
	};

}

