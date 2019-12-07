#pragma once

#include "Path.h"
#include "Asset/Model.h"

#include "DX12CommittedTexture2D.h"

namespace Egg::Importer {
	

	void ImportModel(const char* filePath, Asset::Model & model);

	void ImportModel(const MediaPath & mediaFile, Asset::Model & model);

	//@TODO: refactor this out, into engine
	std::unique_ptr<Graphics::DX12::Resource::ITexture> ImportCommittedTexture2D(ID3D12Device * device, const MediaPath & filePath);


}

