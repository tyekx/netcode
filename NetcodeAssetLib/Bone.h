#pragma once

#include <DirectXMath.h>

namespace Netcode::Asset {

	struct Bone {

		char name[64];
		int parentId;
		DirectX::XMFLOAT4X4 transform;

	};

}
