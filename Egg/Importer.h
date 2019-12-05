#pragma once

#include "Path.h"
#include "Asset/Model.h"

#include "CommittedTexture2D.h"

namespace Egg::Importer {
	

	void ImportModel(const char* filePath, Asset::Model & model);

	void ImportModel(const MediaPath & mediaFile, Asset::Model & model);

	std::unique_ptr<Graphics::Resource::ITexture> ImportCommittedTexture2D(ID3D12Device * device, const MediaPath & filePath);


}

