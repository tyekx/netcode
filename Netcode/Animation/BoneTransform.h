#pragma once

#include <DirectXMath.h>

namespace Netcode::Animation {

	struct BoneTransform {
		DirectX::XMVECTOR translation;
		DirectX::XMVECTOR rotation;
		DirectX::XMVECTOR scale;
	};

	struct BoneAngularLimit {
		DirectX::XMFLOAT3 axis;
		// the bone will be able to move between -limit; +limit angles around the axis
		float angleLimitInRadians;
	};

}
