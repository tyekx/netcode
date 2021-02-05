#pragma once

#include <Netcode/MathExt.h>
#include <Netcode/PhysXWrapper.h>
#include "GameObject.h"

#define PHYSX_COLLIDER_TYPE_WORLD 1
#define PHYSX_COLLIDER_TYPE_LOCAL_HITBOX 2
#define PHYSX_COLLIDER_TYPE_CLIENT_HITBOX 4
#define PHYSX_COLLIDER_TYPE_SERVER_HITBOX 8
#define PHYSX_COLLIDER_TYPE_KILLZONE 16
#define PHYSX_COLLIDER_TYPE_UI 32

class HitscanQueryFilter : physx::PxQueryFilterCallback {
	virtual physx::PxQueryHitType::Enum preFilter(
		const physx::PxFilterData & filterData,
		const physx::PxShape * shape,
		const physx::PxRigidActor * actor,
		physx::PxHitFlags & queryFlags) override
	{
		return physx::PxQueryHitType::Enum::eNONE;
	}

	virtual physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData & filterData, const physx::PxQueryHit & hit) override
	{
		return physx::PxQueryHitType::Enum::eNONE;
	}
};

physx::PxFilterFlags SimulationFilterShader(
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags & pairFlags, const void * constantBlock, physx::PxU32 constantBlockSize);

physx::PxShape * CreatePrimitiveShapeFromAsset(const ColliderShape & cShape, physx::PxPhysics * px, physx::PxMaterial * mat, physx::PxShapeFlags flags, bool isExclusive = true);

physx::PxShape * CreateHitboxShapeFromAsset(const ColliderShape & cShape, physx::PxPhysics * px);

