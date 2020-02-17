#pragma once

#include <Egg/EggMath.h>
#include <Egg/PhysXScene.h>
#include "GameObject.h"

#define PHYSX_COLLIDER_TYPE_WORLD 1
#define PHYSX_COLLIDER_TYPE_HITBOX 2

class HitscanQueryFilter : physx::PxQueryFilterCallback {
	virtual physx::PxQueryHitType::Enum preFilter(
		const physx::PxFilterData & filterData,
		const physx::PxShape * shape,
		const physx::PxRigidActor * actor,
		physx::PxHitFlags & queryFlags) override
	{

	}

	virtual physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData & filterData, const physx::PxQueryHit & hit) override
	{

	}
};

physx::PxShape * CreatePrimitiveShapeFromAsset(ColliderShape & cShape, physx::PxPhysics* px, physx::PxMaterial * mat, physx::PxShapeFlags flags, bool isExclusive = true) {
	physx::PxShape * colliderShape = nullptr;
	switch(cShape.type) {
		case Egg::Asset::ColliderType::CAPSULE:
		{
			physx::PxCapsuleGeometry capsuleGeometry{ cShape.capsuleArgs.y, cShape.capsuleArgs.x / 2.0f };
			colliderShape = px->createShape(capsuleGeometry, *mat, isExclusive, flags);
		}
		break;
		case Egg::Asset::ColliderType::SPHERE:
		{
			physx::PxSphereGeometry sphere{ cShape.sphereArgs };
			colliderShape = px->createShape(sphere, *mat, isExclusive, flags);
		}
		break;
		case Egg::Asset::ColliderType::BOX:
		{
			physx::PxBoxGeometry box{ ToPxVec3(cShape.boxArgs) };
			colliderShape = px->createShape(box, *mat, isExclusive, flags);
		}
	}
	return colliderShape;
}

physx::PxShape * CreateHitboxShapeFromAsset(ColliderShape & cShape, physx::PxPhysics * px) {
	static physx::PxMaterial * dummyMaterial = px->createMaterial(0.1f, 0.1f, 0.1f);
	const auto pxFlags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE | physx::PxShapeFlag::eTRIGGER_SHAPE | physx::PxShapeFlag::eVISUALIZATION;

	auto * colliderShape = CreatePrimitiveShapeFromAsset(cShape, px, dummyMaterial, pxFlags);

	physx::PxTransform pxT{ ToPxVec3(cShape.localPosition), ToPxQuat(cShape.localRotation) };
	colliderShape->setLocalPose(pxT);
	physx::PxFilterData hitboxFilterData;
	hitboxFilterData.word0 = PHYSX_COLLIDER_TYPE_HITBOX;
	hitboxFilterData.word1 = PHYSX_COLLIDER_TYPE_HITBOX;
	colliderShape->setQueryFilterData(hitboxFilterData);
	colliderShape->setSimulationFilterData(hitboxFilterData);
	colliderShape->userData = &cShape;

	return colliderShape;
}

