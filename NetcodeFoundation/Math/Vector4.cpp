#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"

namespace Netcode {

	Vector4::Vector4(const Float4 & rhs) noexcept : v{ DirectX::XMLoadFloat4(&rhs) } {}

	Vector4 & NC_MATH_CALLCONV Vector4::operator=(const Float4 & rhs) noexcept {
		v = DirectX::XMLoadFloat4(&rhs);
		return *this;
	}

	Vector4 NC_MATH_CALLCONV Vector4::operator/(NonZero<Vector4> rhs) const NETCODE_MATH_DEP_NOEXCEPT {
		return DirectX::XMVectorDivide(v, static_cast<Vector4>(rhs).v);
	}

	Vector4 NC_MATH_CALLCONV Vector4::operator/(NonZero<float> scale) const NETCODE_MATH_DEP_NOEXCEPT {
		return DirectX::XMVectorScale(v, 1.0f / scale);
	}

	Vector4 & NC_MATH_CALLCONV Vector4::operator/=(NonZero<Vector4> rhs) NETCODE_MATH_DEP_NOEXCEPT {
		v = DirectX::XMVectorDivide(v, static_cast<Vector4>(rhs).v);
		return *this;
	}

	Vector4 & NC_MATH_CALLCONV Vector4::operator/=(NonZero<float> scale) NETCODE_MATH_DEP_NOEXCEPT {
		v = DirectX::XMVectorScale(v, 1.0f / scale);
		return *this;
	}

	Vector4 NC_MATH_CALLCONV Vector4::Transform(Matrix rhs) const noexcept {
		return Vector4 { DirectX::XMVector4Transform(v, rhs.mat) };
	}

	Vector4 NC_MATH_CALLCONV Vector4::Max(Vector2 rhs) const noexcept {
		return rhs.Max(*this);
	}

	Vector4 NC_MATH_CALLCONV Vector4::Max(Vector3 rhs) const noexcept {
		return rhs.Max(*this);
	}

	Vector4 NC_MATH_CALLCONV Vector4::Max(Vector4 rhs) const noexcept {
		return DirectX::XMVectorMax(v, rhs.v);
	}

	Vector4 NC_MATH_CALLCONV Vector4::Min(Vector2 rhs) const noexcept {
		return rhs.Min(*this);
	}

	Vector4 NC_MATH_CALLCONV Vector4::Min(Vector3 rhs) const noexcept {
		return rhs.Min(*this);
	}

	Vector4 NC_MATH_CALLCONV Vector4::Min(Vector4 rhs) const noexcept {
		return DirectX::XMVectorMin(v, rhs.v);
	}

	float NC_MATH_CALLCONV Vector4::Length() const noexcept {
		return sqrtf(LengthSq());
	}

	float NC_MATH_CALLCONV Vector4::LengthSq() const noexcept {
		return Dot(*this);
	}

	float NC_MATH_CALLCONV Vector4::Dot(Vector4 rhs) const noexcept
	{
		float f;
		DirectX::XMStoreFloat(&f, DirectX::XMVector4Dot(v, rhs.v));
		return f;
	}


	bool NC_MATH_CALLCONV Vector4::AnyZero() const noexcept
	{
		return DirectX::XMComparisonAnyTrue(DirectX::XMVector4EqualR(v, DirectX::g_XMZero));
	}

	bool NC_MATH_CALLCONV Vector4::AllZero() const noexcept
	{
		return DirectX::XMComparisonAllTrue(DirectX::XMVector4EqualR(v, DirectX::g_XMZero));;
	}

	Vector2 NC_MATH_CALLCONV Vector4::XY() const noexcept {
		return Vector2{ v };
	}

	Vector2 NC_MATH_CALLCONV Vector4::ZW() const noexcept {
		return Swizzle<2, 3>();
	}

	Vector3 NC_MATH_CALLCONV Vector4::XYZ() const noexcept {
		return Vector3{ v };
	}


	template<uint32_t COMPONENT0, uint32_t COMPONENT1>
	Vector2 NC_MATH_CALLCONV Vector4::Swizzle() const noexcept {
		return DirectX::XMVectorSwizzle<COMPONENT0, COMPONENT1, 2, 3>(v);
	}

	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<0, 0>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<0, 1>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<0, 2>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<0, 3>() const noexcept;

	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<1, 0>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<1, 1>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<1, 2>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<1, 3>() const noexcept;

	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<2, 0>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<2, 1>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<2, 2>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<2, 3>() const noexcept;

	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<3, 0>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<3, 1>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<3, 2>() const noexcept;
	template Vector2 NC_MATH_CALLCONV Vector4::Swizzle<3, 3>() const noexcept;

	template<uint32_t COMPONENT0, uint32_t COMPONENT1, uint32_t COMPONENT2>
	Vector3 NC_MATH_CALLCONV Vector4::Swizzle() const noexcept {
		return DirectX::XMVectorSwizzle<COMPONENT0, COMPONENT1, COMPONENT2, 3>(v);
	}

	// only instantiate the most common versions for now
	template Vector3 NC_MATH_CALLCONV Vector4::Swizzle<0, 0, 0>() const noexcept;
	template Vector3 NC_MATH_CALLCONV Vector4::Swizzle<1, 1, 1>() const noexcept;
	template Vector3 NC_MATH_CALLCONV Vector4::Swizzle<2, 2, 2>() const noexcept;
	template Vector3 NC_MATH_CALLCONV Vector4::Swizzle<3, 3, 3>() const noexcept;
	template Vector3 NC_MATH_CALLCONV Vector4::Swizzle<0, 1, 2>() const noexcept;



}

