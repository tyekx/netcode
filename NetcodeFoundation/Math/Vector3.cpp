#include "Vector3.h"
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

	Vector4 Vector3::XYZ1() const noexcept {
		return Vector4{ DirectX::XMVectorSelect(DirectX::g_XMOne, v, DirectX::g_XMSelect1110.v) };
	}

	Vector4 Vector3::XYZ0() const noexcept {
		return Vector4{ DirectX::XMVectorSelect(DirectX::g_XMZero, v, DirectX::g_XMSelect1110.v) };
	}

}
