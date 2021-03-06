#include "PhysxHelpers.h"


/*
dont use any global memory
*/
physx::PxFilterFlags SimulationFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags & pairFlags, const void * constantBlock, physx::PxU32 constantBlockSize)
{
	// let triggers through
	if(physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
	{
		constexpr uint32_t NON_INTERACTIBLE =
			PHYSX_COLLIDER_TYPE_CLIENT_HITBOX |
			PHYSX_COLLIDER_TYPE_LOCAL_HITBOX |
			PHYSX_COLLIDER_TYPE_SERVER_HITBOX;

		if((filterData0.word0 & NON_INTERACTIBLE) ||
		   (filterData1.word0 & NON_INTERACTIBLE)) {
			return physx::PxFilterFlag::eSUPPRESS;
		}

		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
		return physx::PxFilterFlag::eDEFAULT;
	}

	pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

	if((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;

	return physx::PxFilterFlag::eDEFAULT;
}

physx::PxShape * CreatePrimitiveShapeFromAsset(const ColliderShape & cShape, physx::PxPhysics * px, physx::PxMaterial * mat, physx::PxShapeFlags flags, bool isExclusive) {
	physx::PxShape * colliderShape = nullptr;
	switch(cShape.type) {
		case Netcode::Asset::ColliderType::CAPSULE:
		{
			physx::PxCapsuleGeometry capsuleGeometry{ cShape.capsuleArgs.y, cShape.capsuleArgs.x / 2.0f };
			colliderShape = px->createShape(capsuleGeometry, *mat, isExclusive, flags);
		}
		break;
		case Netcode::Asset::ColliderType::SPHERE:
		{
			physx::PxSphereGeometry sphere{ cShape.sphereArgs };
			colliderShape = px->createShape(sphere, *mat, isExclusive, flags);
		}
		break;
		case Netcode::Asset::ColliderType::BOX:
		{
			physx::PxBoxGeometry box{ ToPxVec3(cShape.boxArgs) };
			colliderShape = px->createShape(box, *mat, isExclusive, flags);
		}
		default: break;
	}
	return colliderShape;
}

physx::PxShape * CreateHitboxShapeFromAsset(const ColliderShape & cShape, physx::PxPhysics * px) {
	static physx::PxMaterial * dummyMaterial = px->createMaterial(0.1f, 0.1f, 0.1f);
	const auto pxFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eTRIGGER_SHAPE | physx::PxShapeFlag::eVISUALIZATION;

	auto * colliderShape = CreatePrimitiveShapeFromAsset(cShape, px, dummyMaterial, pxFlags);

	physx::PxTransform pxT{ ToPxVec3(cShape.localPosition), ToPxQuat(cShape.localRotation) };
	colliderShape->setLocalPose(pxT);
	physx::PxFilterData hitboxFilterData;
	hitboxFilterData.word0 = PHYSX_COLLIDER_TYPE_CLIENT_HITBOX;
	hitboxFilterData.word1 = PHYSX_COLLIDER_TYPE_CLIENT_HITBOX;
	colliderShape->setQueryFilterData(hitboxFilterData);
	colliderShape->setSimulationFilterData(hitboxFilterData);
	colliderShape->userData = nullptr;

	return colliderShape;
}
