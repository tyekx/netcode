#pragma once

#include "PhysxSystem.h"
#include "GameObject.h"
#include <PxPhysicsAPI.h>

#define PX_RELEASE(ptr) if(ptr) { ptr->release(); ptr = nullptr; }

namespace Egg {

	class PhysxSystem {

	public:
		physx::PxDefaultAllocator allocator;
		physx::PxDefaultErrorCallback errorCallback;
		physx::PxFoundation * foundation;
		physx::PxPhysics * physics;
		physx::PxPvd * debugger;
		physx::PxScene * scene;
		physx::PxDefaultCpuDispatcher * dispatcher;
		physx::PxControllerManager * controllerManager;
		physx::PxRigidStatic * groundPlane;

		constexpr static SignatureType Required() {
			return (0x1ULL << TupleIndexOf<PhysxComponent, COMPONENTS_T>::value);
		}

		constexpr static SignatureType Incompatible() {
			return (0x0ULL);
		}

		PhysxSystem();

		~PhysxSystem();

		void ReleaseResources();

		void CreateResources();

		void Simulate(float dt);

		void Run(Egg::GameObject * gameObject);

	};

}
