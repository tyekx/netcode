#pragma once
#include "Common.h"

namespace Netcode {

	class Vector2 {
	public:
		DirectX::XMVECTOR v;

		DEFAULT_NOEXCEPT_ALL6(Vector2);

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

		Vector2 & NC_MATH_CALLCONV operator+=(Vector2 rhs) noexcept {
			v = DirectX::XMVectorAdd(v, rhs.v);
			return *this;
		}

		Vector2 NC_MATH_CALLCONV operator-(Vector2 rhs) const noexcept {
			return DirectX::XMVectorSubtract(v, rhs.v);
		}

		Vector2 & NC_MATH_CALLCONV operator-=(Vector2 rhs) noexcept {
			v = DirectX::XMVectorSubtract(v, rhs.v);
			return *this;
		}

		bool NC_MATH_CALLCONV AnyZero() const noexcept;

		bool NC_MATH_CALLCONV AllZero() const noexcept;

		NC_MATH_CALLCONV operator Float2() const noexcept {
			Float2 f;
			DirectX::XMStoreFloat2(&f, v);
			return f;
		}
	};

}
