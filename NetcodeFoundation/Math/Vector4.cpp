#include "Vector4.h"
#include "Vector4.h"

namespace Netcode {

	Vector4::Vector4(const Float4 & rhs) noexcept : v{ DirectX::XMLoadFloat4(&rhs) } {}

	Vector4 & Vector4::operator=(const Float4 & rhs) noexcept {
		v = DirectX::XMLoadFloat4(&rhs);
		return *this;
	}

	Vector4 Vector4::Transform(Matrix rhs) const noexcept {
		return Vector4 { DirectX::XMVector4Transform(v, rhs.mat) };
	}

	float NC_MATH_CALLCONV Vector4::Length() const noexcept {
		return sqrtf(LengthSq());
	}

	float NC_MATH_CALLCONV Vector4::LengthSq() const noexcept {
		return Dot(*this);
	}

	float Vector4::Dot(Vector4 rhs) const noexcept
	{
		float f;
		DirectX::XMStoreFloat(&f, DirectX::XMVector4Dot(v, rhs.v));
		return f;
	}

	bool Vector4::AnyZero() const noexcept
	{
		return DirectX::XMComparisonAnyTrue(DirectX::XMVector4EqualR(v, DirectX::g_XMZero));
	}

	bool Vector4::AllZero() const noexcept
	{
		return DirectX::XMComparisonAllTrue(DirectX::XMVector4EqualR(v, DirectX::g_XMZero));;
	}

}
