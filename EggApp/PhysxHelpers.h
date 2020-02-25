#pragma once

#include <Egg/EggMath.h>
#include <Egg/PhysXWrapper.h>
#include "GameObject.h"

#define PHYSX_COLLIDER_TYPE_WORLD 1
#define PHYSX_COLLIDER_TYPE_HITBOX 2
#define PHYSX_COLLIDER_TYPE_KILLZONE 4


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

/*
dont use any global memory
*/
physx::PxFilterFlags SimulationFilterShader(
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags & pairFlags, const void * constantBlock, physx::PxU32 constantBlockSize);

physx::PxShape * CreatePrimitiveShapeFromAsset(ColliderShape & cShape, physx::PxPhysics * px, physx::PxMaterial * mat, physx::PxShapeFlags flags, bool isExclusive = true);

physx::PxShape * CreateHitboxShapeFromAsset(ColliderShape & cShape, physx::PxPhysics * px);

