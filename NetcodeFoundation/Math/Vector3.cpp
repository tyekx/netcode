#include "Vector3.h"
#include "Vector2.h"
#include "Vector4.h"

namespace Netcode {

	Vector3::Vector3(const Float3 & rhs) noexcept : v{ DirectX::XMLoadFloat3(&rhs) } { }

	Vector3::Vector3(const Float4 & rhs) noexcept : v{ DirectX::XMLoadFloat4(&rhs) } {
	}

	Vector3 & Vector3::operator=(Float3 rhs) noexcept {
		v = DirectX::XMLoadFloat3(&rhs);
		return *this;
	}

	Vector3 Vector3::Transform(Matrix rhs) const noexcept {
		return Vector3{ DirectX::XMVector3Transform(v, rhs.mat) };
	}

	Vector3 Vector3::Rotate(Quaternion q) const noexcept {
		return DirectX::XMVector3Rotate(v, q.q);
	}

	Vector3 Vector3::Cross(Vector3 rhs) const noexcept {
		return DirectX::XMVector3Cross(v, rhs.v);
	}

	float Vector3::Dot(Vector3 rhs) const noexcept {
		float f;
		DirectX::XMStoreFloat(&f, DirectX::XMVector3Dot(v, rhs.v));
		return f;
	}

	bool Vector3::AnyZero() const noexcept {
		return DirectX::XMComparisonAnyTrue(DirectX::XMVector3EqualR(v, DirectX::g_XMZero));
	}

	bool Vector3::AllZero() const noexcept {
		return DirectX::XMComparisonAllTrue(DirectX::XMVector3EqualR(v, DirectX::g_XMZero));
	}

	Vector3 NC_MATH_CALLCONV Vector3::Max(Vector2 rhs) const noexcept {
		return rhs.Max(*this);
	}

	Vector3 NC_MATH_CALLCONV Vector3::Max(Vector3 rhs) const noexcept {
		return DirectX::XMVectorMax(v, rhs.v);
	}

	Vector4 NC_MATH_CALLCONV Vector3::Max(Vector4 rhs) const noexcept {
		return DirectX::XMVectorMax(Vector4(v).Permute<0, 1, 2, 7>(rhs).v, rhs.v);
	}

	Vector3 NC_MATH_CALLCONV Vector3::Min(Vector2 rhs) const noexcept {
		return rhs.Min(*this);
	}

	Vector3 NC_MATH_CALLCONV Vector3::Min(Vector3 rhs) const noexcept {
		return DirectX::XMVectorMin(v, rhs.v);
	}

	Vector4 NC_MATH_CALLCONV Vector3::Min(Vector4 rhs) const noexcept {
		return DirectX::XMVectorMin(Vector4(v).Permute<0, 1, 2, 7>(rhs).v, rhs.v);
	}

	Vector4 NC_MATH_CALLCONV Vector3::Extend(float w) const noexcept {
		return DirectX::XMVectorSetByIndex(v, w, 3);
	}

	Vector2 NC_MATH_CALLCONV Vector3::XY() const noexcept {
		return Vector2{ v };
	}

	Vector4 NC_MATH_CALLCONV Vector3::XYZ1() const noexcept {
		return Extend(1.0f);
	}

	Vector4 NC_MATH_CALLCONV Vector3::XYZ0() const noexcept {
		return Extend(0.0f);
	}

	template<uint32_t COMPONENT0, uint32_t COMPONENT1>
	Vector2 NC_MATH_CALLCONV Vector3::Swizzle() const noexcept {
		return DirectX::XMVectorSwizzle<COMPONENT0, COMPONENT1, 2, 3>(v);
	}

	template Vector2 NC_MATH_CALLCONV Vector3::Swizzle<0, 0>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector3::Swizzle<0, 1>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector3::Swizzle<0, 2>() const noexcept;

	template Vector2 NC_MATH_CALLCONV Vector3::Swizzle<1, 0>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector3::Swizzle<1, 1>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector3::Swizzle<1, 2>() const noexcept;

	template Vector2 NC_MATH_CALLCONV Vector3::Swizzle<2, 0>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector3::Swizzle<2, 1>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector3::Swizzle<2, 2>() const noexcept;


	template<uint32_t COMPONENT0, uint32_t COMPONENT1, uint32_t COMPONENT2, uint32_t COMPONENT3>
	Vector4 NC_MATH_CALLCONV Vector3::Swizzle() const noexcept {
		return DirectX::XMVectorSwizzle<COMPONENT0, COMPONENT1, COMPONENT2, COMPONENT3>(v);
	}

	// only instantiate the most common versions for now
	template Vector4 NC_MATH_CALLCONV Vector3::Swizzle<0, 0, 0, 0>() const noexcept;
	template Vector4 NC_MATH_CALLCONV Vector3::Swizzle<1, 1, 1, 1>() const noexcept;
	template Vector4 NC_MATH_CALLCONV Vector3::Swizzle<2, 2, 2, 2>() const noexcept;

	template Vector4 NC_MATH_CALLCONV Vector3::Swizzle<0, 1, 0, 1>() const noexcept;
	template Vector4 NC_MATH_CALLCONV Vector3::Swizzle<1, 2, 1, 2>() const noexcept;
	template Vector4 NC_MATH_CALLCONV Vector3::Swizzle<0, 2, 0, 2>() const noexcept;

	template Vector4 NC_MATH_CALLCONV Vector3::Swizzle<0, 1, 2, 0>() const noexcept;
	template Vector4 NC_MATH_CALLCONV Vector3::Swizzle<0, 1, 2, 1>() const noexcept;
	template Vector4 NC_MATH_CALLCONV Vector3::Swizzle<0, 1, 2, 2>() const noexcept;

}
