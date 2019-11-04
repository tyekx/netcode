#pragma once

#include "Common.h"
#include "Asset/Model.h"

namespace Egg::Graphics {

	class IEngine {
	public:
		virtual ~IEngine() = default;
		
		virtual void CreateResources(void* parentWindow) = 0;
		virtual UINT GetHandle(UINT numberOfItems) = 0;
		virtual Model LoadAssets(UINT handle, Asset::Model * model) = 0;
	};

}
