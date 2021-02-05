#include "Quaternion.h"
#include "Vector3.h"

namespace Netcode {

	Quaternion::Quaternion(Vector3 axis, float angle) noexcept : q{ DirectX::XMQuaternionRotationAxis(axis.v, angle) } { }

	Quaternion NC_MATH_CALLCONV Quaternion::FromPitchYawRollVector(Vector3 v) noexcept {
		return Quaternion{ DirectX::XMQuaternionRotationRollPitchYawFromVector(v.v) };
	}

	Quaternion NC_MATH_CALLCONV Quaternion::FromEulerAngles(const Netcode::Float3 & eulerAngles) noexcept {
		const Netcode::Vector3 constant = Netcode::Float3 {
			Netcode::C_PI / 180.0f,
			Netcode::C_PI / 180.0f,
			Netcode::C_PI / 180.0f,
		};

		return FromPitchYawRollVector(constant * eulerAngles);
	}
	
	Quaternion NC_MATH_CALLCONV Quaternion::FromRotationBetween(Vector3 fromVector, Vector3 toVector, const Float3 & fallbackAxis) noexcept {
		const float uvLen = sqrtf(fromVector.Dot(fromVector) * toVector.Dot(toVector));
		float realPart = uvLen + fromVector.Dot(toVector);

		Float3 axis;
		constexpr float epsilon = 0.00001f;
		if(realPart < epsilon * uvLen) {
			realPart = 0.0f;
			axis = fallbackAxis;
		} else {
			axis = fromVector.Cross(toVector);
		}

		return Quaternion{ Float4{ axis.x, axis.y, axis.z, realPart } }.Normalize();
	}

}
