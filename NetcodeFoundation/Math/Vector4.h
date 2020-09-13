#pragma once
#include "Common.h"
#include "Matrix.h"

namespace Netcode {

	class Vector2;
	class Vector3;

	class Vector4 {
	public:
		DirectX::XMVECTOR v;

		NETCODE_CONSTRUCTORS_ALL(Vector4);

		explicit Vector4(float xyzw) : v{ DirectX::XMVectorReplicate(xyzw) } { }

		inline Vector4(const DirectX::XMVECTOR & v) : v{ v } { }

		Vector4(const Float4 & rhs) noexcept;

		Vector4 & NC_MATH_CALLCONV operator=(const Float4 & rhs) noexcept;

		Vector4 NC_MATH_CALLCONV Transform(Matrix rhs) const noexcept;

		float NC_MATH_CALLCONV Dot(Vector4 rhs) const noexcept;

		Vector4 NC_MATH_CALLCONV operator-() const noexcept {
			return DirectX::XMVectorNegate(v);
		}

		Vector4 NC_MATH_CALLCONV operator*(Vector4 rhs) const noexcept {
			return DirectX::XMVectorMultiply(v, rhs.v);
		}

		Vector4 NC_MATH_CALLCONV operator*(float scale) const noexcept {
			return DirectX::XMVectorScale(v, scale);
		}

		Vector4 NC_MATH_CALLCONV operator/(NonZero<Vector4> rhs) const NETCODE_MATH_DEP_NOEXCEPT;

		Vector4 NC_MATH_CALLCONV operator/(NonZero<float> scale) const NETCODE_MATH_DEP_NOEXCEPT;

		Vector4 & NC_MATH_CALLCONV operator*=(Vector4 rhs) noexcept {
			v = DirectX::XMVectorMultiply(v, rhs.v);
			return *this;
		}

		Vector4 & NC_MATH_CALLCONV operator*=(float scale) noexcept {
			v = DirectX::XMVectorScale(v, scale);
			return *this;
		}

		Vector4 & NC_MATH_CALLCONV operator/=(NonZero<Vector4> rhs) NETCODE_MATH_DEP_NOEXCEPT;

		Vector4 & NC_MATH_CALLCONV operator/=(NonZero<float> scale) NETCODE_MATH_DEP_NOEXCEPT;

		Vector4 NC_MATH_CALLCONV operator+(Vector4 rhs) const noexcept {
			return DirectX::XMVectorAdd(v, rhs.v);
		}

		inline Vector4 NC_MATH_CALLCONV operator+(float rhs) const noexcept {
			return operator+(Vector4{ rhs });
		}

		Vector4 & NC_MATH_CALLCONV operator+=(Vector4 rhs) noexcept {
			v = DirectX::XMVectorAdd(v, rhs.v);
			return *this;
		}

		inline Vector4 & NC_MATH_CALLCONV operator+=(float rhs) noexcept {
			return operator+=(Vector4{ rhs });
		}

		Vector4 NC_MATH_CALLCONV operator-(Vector4 rhs) const noexcept {
			return DirectX::XMVectorSubtract(v, rhs.v);
		}

		inline Vector4 NC_MATH_CALLCONV operator-(float rhs) const noexcept {
			return operator-(Vector4{ rhs });
		}

		Vector4 & NC_MATH_CALLCONV operator-=(Vector4 rhs) noexcept {
			v = DirectX::XMVectorSubtract(v, rhs.v);
			return *this;
		}

		inline Vector4 & NC_MATH_CALLCONV operator-=(float rhs) noexcept {
			return operator-=(Vector4{ rhs });
		}

		static Vector4 NC_MATH_CALLCONV Lerp(Vector4 lhs, Vector4 rhs, float alpha) noexcept {
			return DirectX::XMVectorLerp(lhs.v, rhs.v, alpha);
		}

		Vector4 NC_MATH_CALLCONV Max(Vector2 rhs) const noexcept;

		Vector4 NC_MATH_CALLCONV Max(Vector3 rhs) const noexcept;

		Vector4 NC_MATH_CALLCONV Max(Vector4 rhs) const noexcept;

		Vector4 NC_MATH_CALLCONV Min(Vector2 rhs) const noexcept;

		Vector4 NC_MATH_CALLCONV Min(Vector3 rhs) const noexcept;

		Vector4 NC_MATH_CALLCONV Min(Vector4 rhs) const noexcept;

		float NC_MATH_CALLCONV Length() const noexcept;

		float NC_MATH_CALLCONV LengthSq() const noexcept;

		bool NC_MATH_CALLCONV AnyZero() const noexcept;

		bool NC_MATH_CALLCONV AllZero() const noexcept;

		bool NC_MATH_CALLCONV operator==(Vector4 rhs) const noexcept {
			return DirectX::XMVector4Equal(v, rhs.v);
		}

		inline bool NC_MATH_CALLCONV operator!=(Vector4 rhs) const noexcept {
			return !operator==(rhs);
		}

		bool NC_MATH_CALLCONV NearEqual(Vector4 rhs) const noexcept {
			return DirectX::XMVector4NearEqual(v, rhs.v, DirectX::g_XMEpsilon);
		}

		Vector2 NC_MATH_CALLCONV XY() const noexcept;
		Vector2 NC_MATH_CALLCONV ZW() const noexcept;

		Vector3 NC_MATH_CALLCONV XYZ() const noexcept;

		/*
		COMPONENT0,1 must be between 0-3, otherwise a linker error is produced
		*/
		template<uint32_t COMPONENT0, uint32_t COMPONENT1>
		Vector2 NC_MATH_CALLCONV Swizzle() const noexcept;

		/*
		COMPONENT0,1,2 must be between 0-3, otherwise a linker error is produced
		*/
		template<uint32_t COMPONENT0, uint32_t COMPONENT1, uint32_t COMPONENT2>
		Vector3 NC_MATH_CALLCONV Swizzle() const noexcept;

		template<uint32_t COMPONENT0, uint32_t COMPONENT1, uint32_t COMPONENT2, uint32_t COMPONENT3>
		inline Vector4 NC_MATH_CALLCONV Swizzle() const noexcept {
			return DirectX::XMVectorSwizzle<COMPONENT0, COMPONENT1, COMPONENT2, COMPONENT3>(v);
		}

		template<uint32_t COMPONENT0, uint32_t COMPONENT1, uint32_t COMPONENT2, uint32_t COMPONENT3>
		inline Vector4 NC_MATH_CALLCONV Permute(Vector4 rhs) const noexcept {
			return DirectX::XMVectorPermute<COMPONENT0, COMPONENT1, COMPONENT2, COMPONENT3>(v, rhs.v);
		}

		NC_MATH_CALLCONV operator Float4() const noexcept {
			Float4 f;
			DirectX::XMStoreFloat4(&f, v);
			return f;
		}
	};

	inline Vector4 NC_MATH_CALLCONV operator*(float lhs, const Vector4 & rhs) {
		return rhs.operator*(lhs);
	}

	inline Vector4 NC_MATH_CALLCONV operator/(float lhs, const Vector4 & rhs) {
		return Vector4{ lhs } / rhs;
	}

	inline Vector4 NC_MATH_CALLCONV operator+(float lhs, const Vector4 & rhs) {
		return rhs.operator+(lhs);
	}

	inline Vector4 NC_MATH_CALLCONV operator-(float lhs, const Vector4 & rhs) {
		return Vector4{ lhs } - rhs;
	}

}
