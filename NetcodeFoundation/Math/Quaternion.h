#pragma once
#include "Common.h"
#include "Matrix.h"

namespace Netcode {

	class Vector3;

	class Quaternion {
	public:
		DirectX::XMVECTOR q;

		~Quaternion() noexcept = default;
		Quaternion(const Quaternion & rhs) noexcept = default;
		Quaternion(Quaternion &&) noexcept = default;
		Quaternion & NC_MATH_CALLCONV operator=(const Quaternion & rhs) noexcept = default;
		Quaternion & NC_MATH_CALLCONV operator=(Quaternion && rhs) noexcept = default;

		Quaternion(const Float4 & v) noexcept : q{ DirectX::XMLoadFloat4(&v) } { }
		Quaternion(Vector3 axis, float angle) noexcept;
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

		Quaternion NC_MATH_CALLCONV Conjugate() const noexcept {
			return DirectX::XMQuaternionConjugate(q);
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

		static Quaternion NC_MATH_CALLCONV FromPitchYawRollVector(Vector3 v) noexcept;

		static Quaternion NC_MATH_CALLCONV FromEulerAngles(const Netcode::Float3 & eulerAngles) noexcept;

		/**
		 * Returns the shortest arc quaternion that rotates fromVector to toVector.
		 * @param fallbackAxis used when the vectors are roughly(eps = 0.00001) facing away
		 *
		 * @note implementation is similar Ogre3D's which refers Sam Hocevar's article: http://sam.hocevar.net/blog/2014/02/
		 */
		static Quaternion NC_MATH_CALLCONV FromRotationBetween(Vector3 fromVector, Vector3 toVector, const Float3 & fallbackAxis) noexcept;
	};

	

}
