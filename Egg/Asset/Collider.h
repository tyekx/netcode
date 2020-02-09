#pragma once

#include <DirectXMath.h>

namespace Egg::Asset {

	enum class ColliderType : unsigned {
		UNDEFINED, SPHERE, CAPSULE, BOX, PLANE, MESH
	};

	struct Collider {
		ColliderType type;
		union {
			DirectX::XMFLOAT3 boxArgs;
			DirectX::XMFLOAT3 planeArgs;
			DirectX::XMFLOAT2 capsuleArgs;
			float sphereArgs;
		};
		DirectX::XMFLOAT3 localPosition;
		DirectX::XMFLOAT4 localRotation;
		int32_t boneReference;
	};

}

