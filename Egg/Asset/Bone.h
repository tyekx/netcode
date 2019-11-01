#pragma once

#include <DirectXMath.h>

namespace Egg::Asset {

	struct Bone {

		char name[28];
		int parentId;
		DirectX::XMFLOAT4X4 transform;

	};

}
