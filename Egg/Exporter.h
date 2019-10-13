#pragma once

#include "Asset/Model.h"

namespace Egg {

	namespace Exporter {

		void ExportModel(const char* path, const Asset::Model & m);

		unsigned int CalculateTotalSize(const Asset::Model & m);

	}

}
