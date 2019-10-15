#pragma once

#include <DirectXMath.h>

namespace Egg {

	namespace Asset {

		struct Bone {

			char name[28];
			int parentId;
			DirectX::XMFLOAT4X4 transform;

		};

	}

}
