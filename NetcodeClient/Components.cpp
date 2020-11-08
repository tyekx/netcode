#include "Components.h"
#include <Netcode/MathExt.h>

Transform::Transform() : position{ 0.0f, 0.0f, 0.0f }, rotation{ 0.0f, 0.0f, 0.0f, 1.0f }, scale{ 1.0f, 1.0f, 1.0f }, worldPosition{ position }, worldRotation{ rotation } {

}

physx::PxTransform Transform::LocalToPhysX() const {
	return physx::PxTransform{
		Netcode::ToPxVec3(position),
		Netcode::ToPxQuat(rotation)
	};
}

physx::PxTransform Transform::WorldToPhysX() const {
	return physx::PxTransform{
		Netcode::ToPxVec3(worldPosition),
		Netcode::ToPxQuat(worldRotation)
	};
}

uint32_t ClientAction::idGen{ 1 };
