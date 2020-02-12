#pragma once

#include "Asset/Model.h"

namespace Egg {

	namespace Exporter {

		uint32_t CalculateAnimDataSize(const Asset::Model & m);

		uint32_t CalculateMeshesSize(const Asset::Model & m);

		uint32_t CalculateMaterialsSize(const Asset::Model & m);

		uint32_t CalculateCollidersSize(const Asset::Model & m);

		uint32_t CalculateBonesSize(const Asset::Model & m);

		uint32_t ExportModel(const char* path, const Asset::Model & m);

		uint32_t CalculateTotalSize(const Asset::Model & m);

	}

}
