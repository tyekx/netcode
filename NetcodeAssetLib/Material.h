#pragma once

#include <NetcodeFoundation/Math.h>
#include <NetcodeFoundation/ArrayView.hpp>

namespace Netcode::Asset {

	struct MaterialParamIndex {
		uint32_t id;
		uint16_t size;
		uint16_t offset;
	};

	struct Material {
		char name[48];
		uint32_t type;
		uint32_t indicesLength;
		uint64_t dataSizeInBytes;
		MaterialParamIndex * indices;
		uint8_t * data;
	};

}
