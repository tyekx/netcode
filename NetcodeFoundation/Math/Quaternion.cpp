#include "Quaternion.h"
#include "Vector3.h"

namespace Netcode {

	Quaternion::Quaternion(Vector3 axis, float angle) noexcept : q{ DirectX::XMQuaternionRotationAxis(axis.v, angle) } { }

}
