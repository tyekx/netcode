#pragma once

#include <DirectXMath.h>

namespace Netcode::Animation {

	struct BoneTransform {
		DirectX::XMVECTOR translation;
		DirectX::XMVECTOR rotation;
		DirectX::XMVECTOR scale;
	};

}
