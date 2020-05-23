#pragma once
#include "Common.h"
#include "Matrix.h"

namespace Netcode {

	class Quaternion {
	public:
		DirectX::XMVECTOR q;

		~Quaternion() noexcept = default;
		Quaternion(const Quaternion & rhs) noexcept = default;
		Quaternion(Quaternion &&) noexcept = default;
		Quaternion & NC_MATH_CALLCONV operator=(const Quaternion & rhs) noexcept = default;
		Quaternion & NC_MATH_CALLCONV operator=(Quaternion && rhs) noexcept = default;

		Quaternion(const Float4 & v) noexcept : q{ DirectX::XMLoadFloat4(&v) } { }
		Quaternion(const Float3 & axis, float angle) noexcept : q{ DirectX::XMQuaternionRotationAxis(DirectX::XMLoadFloat3(&axis), angle) } { }
		Quaternion(float pitch, float yaw, float roll) noexcept : q{ DirectX::XMQuaternionRotationRollPitchYaw(pitch, yaw, roll) } { }

		Quaternion(const DirectX::XMVECTOR & q) noexcept : q{ q } { }
		Quaternion() noexcept : q{ DirectX::XMQuaternionIdentity() } { }

		Quaternion & operator=(const Float4 & rhs) noexcept {
			q = DirectX::XMLoadFloat4(&rhs);
			return *this;
		}

		float NC_MATH_CALLCONV Dot(Quaternion rhs) const noexcept {
			float f;
			DirectX::XMStoreFloat(&f, DirectX::XMVector4Dot(q, rhs.q));
			return f;
		}

		Quaternion NC_MATH_CALLCONV operator*(Quaternion rhs) const noexcept {
			return DirectX::XMQuaternionMultiply(q, rhs.q);
		}

		Quaternion NC_MATH_CALLCONV Normalize() const noexcept {
			return DirectX::XMQuaternionNormalize(q);
		}

		static Quaternion NC_MATH_CALLCONV Slerp(Quaternion lhs, Quaternion rhs, float t) noexcept {
			return DirectX::XMQuaternionSlerp(lhs.q, rhs.q, t);
		}

		explicit NC_MATH_CALLCONV operator Matrix() const noexcept {
			return DirectX::XMMatrixRotationQuaternion(q);
		}

		float NC_MATH_CALLCONV LengthSq() const noexcept {
			return Dot(*this);
		}

		NC_MATH_CALLCONV operator Float4() const noexcept {
			Float4 f;
			DirectX::XMStoreFloat4(&f, q);
			return f;
		}
	};

	

}
