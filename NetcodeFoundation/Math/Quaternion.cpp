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

		return Netcode::Quaternion::FromPitchYawRollVector(constant * eulerAngles);
	}

}
