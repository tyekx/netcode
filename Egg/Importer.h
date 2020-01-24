#pragma once

#include "Path.h"
#include "Asset/Model.h"

namespace Egg::Importer {
	

	void ImportModel(const char* filePath, Asset::Model & model);

	void ImportModel(const MediaPath & mediaFile, Asset::Model & model);


}

