#pragma once
#include "Common.h"
#include "Matrix.h"
#include "Quaternion.h"

namespace Netcode {

	class Vector2;
	class Vector4;

	class Vector3 {
	public:
		DirectX::XMVECTOR v;

		DEFAULT_NOEXCEPT_ALL6(Vector3);

		explicit Vector3(float xyzw) : v{ DirectX::XMVectorReplicate(xyzw) } { }

		inline Vector3(const DirectX::XMVECTOR & v) : v{ v } { }

		Vector3(const Float3 & rhs) noexcept;
		Vector3(const Float4 & rhs) noexcept;

		Vector3 & NC_MATH_CALLCONV operator=(Float3 rhs) noexcept;

		Vector3 NC_MATH_CALLCONV Transform(Matrix rhs) const noexcept;

		Vector3 NC_MATH_CALLCONV Rotate(Quaternion q) const noexcept;

		Vector3 NC_MATH_CALLCONV Cross(Vector3 rhs) const noexcept;

		float NC_MATH_CALLCONV Dot(Vector3 rhs) const noexcept;

		static Vector3 NC_MATH_CALLCONV Lerp(Vector3 lhs, Vector3 rhs, float t) noexcept {
			return DirectX::XMVectorLerp(lhs.v, rhs.v, t);
		}

		Vector3 NC_MATH_CALLCONV operator-() const noexcept {
			return DirectX::XMVectorNegate(v);
		}

		Vector3 NC_MATH_CALLCONV operator*(Vector3 rhs) const noexcept {
			return DirectX::XMVectorMultiply(v, rhs.v);
		}

		Vector3 NC_MATH_CALLCONV operator*(float scale) const noexcept {
			return DirectX::XMVectorScale(v, scale);
		}

		Vector3 NC_MATH_CALLCONV operator/(NonZero<Vector3> rhs) const NETCODE_MATH_DEP_NOEXCEPT {
			return DirectX::XMVectorDivide(v, static_cast<Vector3>(rhs).v);
		}

		Vector3 NC_MATH_CALLCONV operator/(NonZero<float> scale) const NETCODE_MATH_DEP_NOEXCEPT {
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

		Vector3 & NC_MATH_CALLCONV operator/=(NonZero<Vector3> rhs) NETCODE_MATH_DEP_NOEXCEPT {
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

		inline Vector3 NC_MATH_CALLCONV operator+(float rhs) const noexcept {
			return operator+(Vector3{ rhs });
		}

		Vector3 & NC_MATH_CALLCONV operator+=(Vector3 rhs) noexcept {
			v = DirectX::XMVectorAdd(v, rhs.v);
			return *this;
		}

		inline Vector3 & NC_MATH_CALLCONV operator+=(float rhs) noexcept {
			return operator+=(Vector3{ rhs });
		}

		Vector3 NC_MATH_CALLCONV operator-(Vector3 rhs) const noexcept {
			return DirectX::XMVectorSubtract(v, rhs.v);
		}

		inline Vector3 NC_MATH_CALLCONV operator-(float rhs) const noexcept {
			return operator-(Vector3{ rhs });
		}

		Vector3& NC_MATH_CALLCONV operator-=(Vector3 rhs) noexcept {
			v = DirectX::XMVectorSubtract(v, rhs.v);
			return *this;
		}

		inline Vector3 & NC_MATH_CALLCONV operator-=(float rhs) noexcept {
			return operator-=(Vector3{ rhs });
		}

		float NC_MATH_CALLCONV Length() const noexcept {
			return sqrtf(LengthSq());
		}

		float NC_MATH_CALLCONV LengthSq() const noexcept {
			return Dot(*this);
		}

		Vector3 NC_MATH_CALLCONV Normalize() const NETCODE_MATH_DEP_NOEXCEPT {
			float len = sqrtf(LengthSq());

			return (*this) / len;
		}

		bool NC_MATH_CALLCONV AnyZero() const noexcept;

		bool NC_MATH_CALLCONV AllZero() const noexcept;

		bool NC_MATH_CALLCONV operator==(Vector3 rhs) const noexcept {
			return DirectX::XMVector3Equal(v, rhs.v);
		}

		inline bool NC_MATH_CALLCONV operator!=(Vector3 rhs) const noexcept {
			return !operator==(rhs);
		}

		bool NC_MATH_CALLCONV NearEqual(Vector3 rhs) const noexcept {
			return DirectX::XMVector3NearEqual(v, rhs.v, DirectX::g_XMEpsilon);
		}

		NC_MATH_CALLCONV operator Float3() const noexcept {
			Float3 f;
			DirectX::XMStoreFloat3(&f, v);
			return f;
		}

		Vector3 NC_MATH_CALLCONV Max(Vector2 rhs) const noexcept;
		Vector3 NC_MATH_CALLCONV Max(Vector3 rhs) const noexcept;
		Vector4 NC_MATH_CALLCONV Max(Vector4 rhs) const noexcept;

		Vector3 NC_MATH_CALLCONV Min(Vector2 rhs) const noexcept;
		Vector3 NC_MATH_CALLCONV Min(Vector3 rhs) const noexcept;
		Vector4 NC_MATH_CALLCONV Min(Vector4 rhs) const noexcept;

		Vector4 NC_MATH_CALLCONV Extend(float w) const noexcept;

		Vector2 NC_MATH_CALLCONV XY() const noexcept;
		Vector4 NC_MATH_CALLCONV XYZ1() const noexcept;
		Vector4 NC_MATH_CALLCONV XYZ0() const noexcept;

		/*
		Permutes between 2 Vector3-s to create a new Vector3.
		Components can be indexed: [0,5] inclusive.
		From [0,2] inclusive, the first vector's components are selected
		From [3,5] inclusive, the second vector's components are selected
		*/
		template<uint32_t COMPONENT0, uint32_t COMPONENT1, uint32_t COMPONENT2>
		Vector3 NC_MATH_CALLCONV Permute(Vector3 rhs) const noexcept {
			static_assert(COMPONENT0 < 6 && COMPONENT1 < 6 && COMPONENT2 < 6, "Out of range");
			constexpr uint32_t c0 = (COMPONENT0 / 3) * 4 + (COMPONENT0 % 3);
			constexpr uint32_t c1 = (COMPONENT1 / 3) * 4 + (COMPONENT1 % 3);
			constexpr uint32_t c2 = (COMPONENT2 / 3) * 4 + (COMPONENT2 % 3);
			return DirectX::XMVectorPermute<c0, c1, c2, 3>(v, rhs.v);
		}

		template<uint32_t COMPONENT0, uint32_t COMPONENT1, uint32_t COMPONENT2>
		Vector3 NC_MATH_CALLCONV Swizzle() const noexcept {
			return DirectX::XMVectorSwizzle<COMPONENT0, COMPONENT1, COMPONENT2, 3>(v);
		}

		template<uint32_t COMPONENT0, uint32_t COMPONENT1>
		Vector2 NC_MATH_CALLCONV Swizzle() const noexcept;

		template<uint32_t COMPONENT0, uint32_t COMPONENT1, uint32_t COMPONENT2, uint32_t COMPONENT3>
		Vector4 NC_MATH_CALLCONV Swizzle() const noexcept;
	};

	inline Vector3 NC_MATH_CALLCONV operator*(float lhs, const Vector3 & rhs) {
		return rhs.operator*(lhs);
	}

	inline Vector3 NC_MATH_CALLCONV operator/(float lhs, const Vector3 & rhs) {
		return Vector3{ lhs } / rhs;
	}

	inline Vector3 NC_MATH_CALLCONV operator+(float lhs, const Vector3 & rhs) {
		return rhs.operator+(lhs);
	}

	inline Vector3 NC_MATH_CALLCONV operator-(float lhs, const Vector3 & rhs) {
		return Vector3{ lhs } - rhs;
	}

}
