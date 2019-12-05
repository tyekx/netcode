#pragma once

#include "Asset/Model.h"

namespace Egg {

	namespace Exporter {

		unsigned int CalculateAnimDataSize(const Asset::Model & m);

		unsigned int CalculateMeshesSize(const Asset::Model & m);

		unsigned int CalculateMaterialsSize(const Asset::Model & m);

		void ExportModel(const char* path, const Asset::Model & m);

		unsigned int CalculateTotalSize(const Asset::Model & m);

	}

}
