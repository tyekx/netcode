#pragma once

#include <NetcodeFoundation/Math.h>

namespace Netcode::Animation {

	struct BoneTransform {
		Netcode::Vector3 translation;
		Netcode::Quaternion rotation;
		Netcode::Vector3 scale;
	};

	struct BoneAngularLimit {
		Netcode::Float3 axis;
		// the bone will be able to move between -limit; +limit angles around the axis
		float angleLimitInRadians;
	};

}
