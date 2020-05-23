#pragma once
#include "Common.h"
#include "Matrix.h"
#include "Quaternion.h"

namespace Netcode {

	class Vector4;

	class Vector3 {
	public:
		DirectX::XMVECTOR v;

		DEFAULT_NOEXCEPT_ALL6(Vector3);

		inline Vector3(const DirectX::XMVECTOR & v) : v{ v } { }

		Vector3(const Float3 & rhs) noexcept;
		Vector3(const Float4 & rhs) noexcept;

		Vector3 & NC_MATH_CALLCONV operator=(Float3 rhs) noexcept;

		Vector3 NC_MATH_CALLCONV Transform(Matrix rhs) const noexcept;

		Vector3 NC_MATH_CALLCONV Rotate(Quaternion q) const noexcept;

		Vector3 NC_MATH_CALLCONV Cross(Vector3 rhs) const noexcept;

		float NC_MATH_CALLCONV Dot(const Vector3 & rhs) const noexcept;

		static Vector3 NC_MATH_CALLCONV Lerp(Vector3 lhs, Vector3 rhs, float t) noexcept {
			return DirectX::XMVectorLerp(lhs.v, rhs.v, t);
		}

		Vector3 NC_MATH_CALLCONV operator*(Vector3 rhs) const noexcept {
			return DirectX::XMVectorMultiply(v, rhs.v);
		}

		Vector3 NC_MATH_CALLCONV operator*(float scale) const noexcept {
			return DirectX::XMVectorScale(v, scale);
		}

		Vector3 NC_MATH_CALLCONV operator/(NonZero<Vector3> rhs) const NETCODE_DEP_NOEXCEPT {
			return DirectX::XMVectorDivide(v, static_cast<Vector3>(rhs).v);
		}

		Vector3 NC_MATH_CALLCONV operator/(NonZero<float> scale) const NETCODE_DEP_NOEXCEPT {
			return DirectX::XMVectorScale(v, 1.0f / scale);
		}

		Vector3 & NC_MATH_CALLCONV operator*=(Vector3 rhs) noexcept {
			v = DirectX::XMVectorMultiply(v, rhs.v);
			return *this;
		}

		Vector3 & NC_MATH_CALLCONV operator*=(float scale) noexcept {
			v = DirectX::XMVectorScale(v, scale);
			return *this;
		}

		Vector3 & NC_MATH_CALLCONV operator/=(NonZero<Vector3> rhs) NETCODE_DEP_NOEXCEPT {
			v = DirectX::XMVectorDivide(v, static_cast<Vector3>(rhs).v);
			return *this;
		}

		Vector3 & NC_MATH_CALLCONV operator/=(NonZero<float> scale) noexcept {
			v = DirectX::XMVectorScale(v, 1.0f / scale);
			return *this;
		}

		Vector3 NC_MATH_CALLCONV operator+(Vector3 rhs) const noexcept {
			return DirectX::XMVectorAdd(v, rhs.v);
		}

		Vector3 & NC_MATH_CALLCONV operator+=(Vector3 rhs) noexcept {
			v = DirectX::XMVectorAdd(v, rhs.v);
			return *this;
		}

		Vector3 NC_MATH_CALLCONV operator-(Vector3 rhs) const noexcept {
			return DirectX::XMVectorSubtract(v, rhs.v);
		}

		Vector3& NC_MATH_CALLCONV operator-=(Vector3 rhs) noexcept {
			v = DirectX::XMVectorSubtract(v, rhs.v);
			return *this;
		}

		float NC_MATH_CALLCONV LengthSq() const noexcept {
			return Dot(*this);
		}

		Vector3 NC_MATH_CALLCONV Normalize() const NETCODE_DEP_NOEXCEPT {
			float len = sqrtf(LengthSq());

			return (*this) / len;
		}

		bool NC_MATH_CALLCONV AnyZero() const noexcept;

		bool NC_MATH_CALLCONV AllZero() const noexcept;

		NC_MATH_CALLCONV operator Float3() const noexcept {
			Float3 f;
			DirectX::XMStoreFloat3(&f, v);
			return f;
		}

		Vector4 XYZ1() const noexcept;

		Vector4 XYZ0() const noexcept;

	};

}
