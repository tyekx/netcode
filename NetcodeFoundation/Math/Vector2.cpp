#include "Vector2.h"

namespace Netcode {

	Vector2::Vector2(const Float2 & rhs) noexcept : v{ DirectX::XMLoadFloat2(&rhs) }
	{
	}

	Vector2 & Vector2::operator=(const Float2 & rhs) noexcept
	{
		v = DirectX::XMLoadFloat2(&rhs);
		return *this;
	}

	bool Vector2::AnyZero() const noexcept
	{
		return DirectX::XMComparisonAnyTrue(DirectX::XMVector2EqualR(v, DirectX::g_XMZero));
	}

	bool Vector2::AllZero() const noexcept
	{
		return DirectX::XMComparisonAllTrue(DirectX::XMVector2EqualR(v, DirectX::g_XMZero));
	}
}
