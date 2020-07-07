#pragma once
#include "Common.h"
#include "Matrix.h"

namespace Netcode {

	class Vector4 {
	public:
		DirectX::XMVECTOR v;

		DEFAULT_NOEXCEPT_ALL6(Vector4);

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

		Vector4 & NC_MATH_CALLCONV operator+=(Vector4 rhs) noexcept {
			v = DirectX::XMVectorAdd(v, rhs.v);
			return *this;
		}

		Vector4 NC_MATH_CALLCONV operator-(Vector4 rhs) const noexcept {
			return DirectX::XMVectorSubtract(v, rhs.v);
		}

		Vector4 & NC_MATH_CALLCONV operator-=(Vector4 rhs) noexcept {
			v = DirectX::XMVectorSubtract(v, rhs.v);
			return *this;
		}

		static Vector4 NC_MATH_CALLCONV Lerp(Vector4 lhs, Vector4 rhs, float alpha) noexcept {
			return DirectX::XMVectorLerp(lhs.v, rhs.v, alpha);
		}

		float NC_MATH_CALLCONV Length() const noexcept;

		float NC_MATH_CALLCONV LengthSq() const noexcept;

		bool NC_MATH_CALLCONV AnyZero() const noexcept;

		bool NC_MATH_CALLCONV AllZero() const noexcept;

		NC_MATH_CALLCONV operator Float4() const noexcept {
			Float4 f;
			DirectX::XMStoreFloat4(&f, v);
			return f;
		}
	};

}
