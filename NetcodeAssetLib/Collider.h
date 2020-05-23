#pragma once

#include <NetcodeFoundation/Math.h>

namespace Netcode::Asset {

	enum class ColliderType : unsigned {
		UNDEFINED, SPHERE, CAPSULE, BOX, MESH, PLANE
	};

	struct Collider {
		ColliderType type;
		union {
			Netcode::Float3 boxArgs;
			Netcode::Float3 planeArgs;
			Netcode::Float2 capsuleArgs;
			float sphereArgs;
		};
		Netcode::Float3 localPosition;
		Netcode::Float4 localRotation;
		int32_t boneReference;
	};

}

