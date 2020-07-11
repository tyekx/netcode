#pragma once
#include "Common.h"

namespace Netcode {

	class Vector3;
	class Vector4;

	class Vector2 {
	public:
		DirectX::XMVECTOR v;

		DEFAULT_NOEXCEPT_ALL6(Vector2);

		explicit Vector2(float xyzw) : v{ DirectX::XMVectorReplicate(xyzw) } { }

		inline Vector2(const DirectX::XMVECTOR & v) : v{ v } { }

		Vector2(const Float2 & rhs) noexcept;

		Vector2 & NC_MATH_CALLCONV operator=(const Float2 & rhs) noexcept;

		Vector2 NC_MATH_CALLCONV operator*(Vector2 rhs) const noexcept {
			return DirectX::XMVectorMultiply(v, rhs.v);
		}

		Vector2 NC_MATH_CALLCONV operator*(float scale) const noexcept {
			return DirectX::XMVectorScale(v, scale);
		}

		Vector2 NC_MATH_CALLCONV operator/(NonZero<Vector2> rhs) const NETCODE_MATH_DEP_NOEXCEPT {
			return DirectX::XMVectorDivide(v, static_cast<Vector2>(rhs).v);
		}

		Vector2 NC_MATH_CALLCONV operator/(NonZero<float> scale) const NETCODE_MATH_DEP_NOEXCEPT {
			return DirectX::XMVectorScale(v, 1.0f / scale);
		}

		Vector2 & NC_MATH_CALLCONV operator*=(Vector2 rhs) noexcept {
			v = DirectX::XMVectorMultiply(v, rhs.v);
			return *this;
		}

		Vector2 & NC_MATH_CALLCONV operator*=(float scale) noexcept {
			v = DirectX::XMVectorScale(v, scale);
			return *this;
		}

		Vector2 & NC_MATH_CALLCONV operator/=(NonZero<Vector2> rhs) NETCODE_MATH_DEP_NOEXCEPT {
			v = DirectX::XMVectorDivide(v, static_cast<Vector2>(rhs).v);
			return *this;
		}

		Vector2 & NC_MATH_CALLCONV operator/=(NonZero<float> scale) noexcept {
			v = DirectX::XMVectorScale(v, 1.0f / scale);
			return *this;
		}

		Vector2 NC_MATH_CALLCONV operator+(Vector2 rhs) const noexcept {
			return DirectX::XMVectorAdd(v, rhs.v);
		}

		inline Vector2 NC_MATH_CALLCONV operator+(float rhs) const noexcept {
			return operator+(Vector2{ rhs });
		}

		Vector2 & NC_MATH_CALLCONV operator+=(Vector2 rhs) noexcept {
			v = DirectX::XMVectorAdd(v, rhs.v);
			return *this;
		}

		inline Vector2 & NC_MATH_CALLCONV operator+=(float rhs) noexcept {
			return operator+=(Vector2{ rhs });
		}

		Vector2 NC_MATH_CALLCONV operator-(Vector2 rhs) const noexcept {
			return DirectX::XMVectorSubtract(v, rhs.v);
		}

		inline Vector2 NC_MATH_CALLCONV operator-(float rhs) const noexcept {
			return operator-(Vector2{ rhs });
		}

		Vector2 NC_MATH_CALLCONV operator-() const noexcept {
			return DirectX::XMVectorNegate(v);
		}

		Vector2 & NC_MATH_CALLCONV operator-=(Vector2 rhs) noexcept {
			v = DirectX::XMVectorSubtract(v, rhs.v);
			return *this;
		}

		inline Vector2 & NC_MATH_CALLCONV operator-=(float rhs) noexcept {
			return operator-=(Vector2{ rhs });
		}

		Vector2 NC_MATH_CALLCONV RotateZ(float radians) const {
			return DirectX::XMVector3Rotate(v, DirectX::XMQuaternionRotationRollPitchYaw(0.0f, 0.0f, radians));
		}

		Vector2 NC_MATH_CALLCONV Reciprocal() const {
			NonZero<Vector2> instanceIsNotNull{ *this };
			return DirectX::XMVectorReciprocal(v);
		}

		Vector2 NC_MATH_CALLCONV Max(Vector2 rhs) const noexcept {
			return DirectX::XMVectorMax(v, rhs.v);
		}

		Vector2 NC_MATH_CALLCONV Min(Vector2 rhs) const noexcept {
			return DirectX::XMVectorMin(v, rhs.v);
		}

		Vector3 NC_MATH_CALLCONV Max(Vector3 rhs) const noexcept;

		Vector4 NC_MATH_CALLCONV Max(Vector4 rhs) const noexcept;

		Vector3 NC_MATH_CALLCONV Min(Vector3 rhs) const noexcept;

		Vector4 NC_MATH_CALLCONV Min(Vector4 rhs) const noexcept;

		Vector3 NC_MATH_CALLCONV Extend(float z) const noexcept;

		Vector4 NC_MATH_CALLCONV Extend(Vector2 rhs) const noexcept;

		Vector4 NC_MATH_CALLCONV Extend(float z, float w) const noexcept;

		bool NC_MATH_CALLCONV AnyZero() const noexcept;

		bool NC_MATH_CALLCONV AllZero() const noexcept;

		bool NC_MATH_CALLCONV operator==(Vector2 rhs) const noexcept {
			return DirectX::XMVector2Equal(v, rhs.v);
		}

		inline bool NC_MATH_CALLCONV operator!=(Vector2 rhs) const noexcept {
			return !operator==(rhs);
		}

		bool NC_MATH_CALLCONV NearEqual(Vector2 rhs) const noexcept {
			return DirectX::XMVector2NearEqual(v, rhs.v, DirectX::g_XMEpsilon);
		}

		NC_MATH_CALLCONV operator Float2() const noexcept {
			Float2 f;
			DirectX::XMStoreFloat2(&f, v);
			return f;
		}

		template<uint32_t COMPONENT0, uint32_t COMPONENT1, uint32_t COMPONENT2, uint32_t COMPONENT3>
		Vector4 NC_MATH_CALLCONV Swizzle() const noexcept;
	};

	inline Vector2 NC_MATH_CALLCONV operator*(float lhs, const Vector2 & rhs) {
		return rhs.operator*(lhs);
	}

	inline Vector2 NC_MATH_CALLCONV operator/(float lhs, const Vector2 & rhs) {
		return Vector2{ lhs } / rhs;
	}

	inline Vector2 NC_MATH_CALLCONV operator+(float lhs, const Vector2 & rhs) {
		return rhs.operator+(lhs);
	}

	inline Vector2 NC_MATH_CALLCONV operator-(float lhs, const Vector2 & rhs) {
		return Vector2{ lhs } - rhs;
	}

}
