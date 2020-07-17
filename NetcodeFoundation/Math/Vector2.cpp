#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

namespace Netcode {

	Vector2::Vector2(const Float2 & rhs) noexcept : v{ DirectX::XMLoadFloat2(&rhs) }
	{
	}

	Vector2 & NC_MATH_CALLCONV Vector2::operator=(const Float2 & rhs) noexcept
	{
		v = DirectX::XMLoadFloat2(&rhs);
		return *this;
	}

	Vector3 NC_MATH_CALLCONV Vector2::Extend(float z) const noexcept {
		return DirectX::XMVectorSetByIndex(v, z, 2);
	}

	Vector4 NC_MATH_CALLCONV Vector2::Extend(Vector2 rhs) const noexcept {
		return DirectX::XMVectorPermute<0, 1, 4, 5>(v, rhs.v);
	}

	Vector4 NC_MATH_CALLCONV Vector2::Extend(float z, float w) const noexcept {
		return Extend(Float2{ z, w });
	}

	bool NC_MATH_CALLCONV Vector2::AnyZero() const noexcept
	{
		return DirectX::XMComparisonAnyTrue(DirectX::XMVector2EqualR(v, DirectX::g_XMZero));
	}

	bool NC_MATH_CALLCONV Vector2::AllZero() const noexcept
	{
		return DirectX::XMComparisonAllTrue(DirectX::XMVector2EqualR(v, DirectX::g_XMZero));
	}

	Vector3 NC_MATH_CALLCONV Vector2::Max(Vector3 rhs) const noexcept
	{
		return DirectX::XMVectorMax(Vector3(v).Permute<0, 1, 5>(rhs).v, rhs.v);
	}

	Vector4 NC_MATH_CALLCONV Vector2::Max(Vector4 rhs) const noexcept
	{
		return DirectX::XMVectorMax(Vector4(v).Permute<0, 1, 6, 7>(rhs).v, rhs.v);
	}

	Vector3 NC_MATH_CALLCONV Vector2::Min(Vector3 rhs) const noexcept {
		return DirectX::XMVectorMin(Vector3(v).Permute<0, 1, 5>(rhs).v, rhs.v);
	}

	Vector4 NC_MATH_CALLCONV Vector2::Min(Vector4 rhs) const noexcept {
		return DirectX::XMVectorMin(Vector4(v).Permute<0, 1, 6, 7>(rhs).v, rhs.v);
	}

	template<uint32_t COMPONENT0, uint32_t COMPONENT1>
	Vector2 NC_MATH_CALLCONV Vector2::Swizzle() const noexcept {
		return DirectX::XMVectorSwizzle<COMPONENT0, COMPONENT1, 2, 3>(v);
	}

	template<uint32_t COMPONENT0, uint32_t COMPONENT1, uint32_t COMPONENT2>
	Vector3 NC_MATH_CALLCONV Vector2::Swizzle() const noexcept {
		return DirectX::XMVectorSwizzle<COMPONENT0, COMPONENT1, COMPONENT2, 3>(v);
	}

	template<uint32_t COMPONENT0, uint32_t COMPONENT1, uint32_t COMPONENT2, uint32_t COMPONENT3>
	Vector4 Vector2::Swizzle() const noexcept {
		return DirectX::XMVectorSwizzle<COMPONENT0, COMPONENT1, COMPONENT2, COMPONENT3>(v);
	}

	template Vector2 NC_MATH_CALLCONV Vector2::Swizzle<0, 0>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector2::Swizzle<0, 1>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector2::Swizzle<1, 0>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector2::Swizzle<1, 1>() const noexcept;

	template Vector3 NC_MATH_CALLCONV Vector2::Swizzle<0, 0, 0>() const noexcept;
	template Vector3 NC_MATH_CALLCONV Vector2::Swizzle<1, 1, 1>() const noexcept;
	template Vector3 NC_MATH_CALLCONV Vector2::Swizzle<0, 1, 0>() const noexcept;
	template Vector3 NC_MATH_CALLCONV Vector2::Swizzle<0, 1, 1>() const noexcept;

	template Vector4 NC_MATH_CALLCONV Vector2::Swizzle<0, 0, 0, 0>() const noexcept;
	template Vector4 NC_MATH_CALLCONV Vector2::Swizzle<0, 1, 0, 1>() const noexcept;
	template Vector4 NC_MATH_CALLCONV Vector2::Swizzle<1, 1, 1, 1>() const noexcept;

}