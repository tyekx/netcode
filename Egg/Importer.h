#pragma once

#include "Mesh/Geometry.h"
#include "Texture2D.h"
#include "Path.h"

namespace Egg {
	namespace Importer {

		Egg::Mesh::Geometry::P ImportSimpleObj(ID3D12Device * device, const MediaPath & filePath);

		Texture2D ImportTexture2D(ID3D12Device * device, const MediaPath & filePath);

	};
}

