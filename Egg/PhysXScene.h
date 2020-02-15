#pragma once

#include "Modules.h"
#include <physx/PxPhysicsAPI.h>
#include <vector>

#define PX_RELEASE(ptr) if(ptr) { ptr->release(); ptr = nullptr; }

namespace Egg::Physics {

	class PhysXScene {
		physx::PxDefaultAllocator allocator;
		physx::PxDefaultErrorCallback errorCallback;
		physx::PxFoundation * foundation;
		physx::PxPhysics * physics;
		physx::PxPvd * debugger;
		physx::PxScene * scene;
		physx::PxDefaultCpuDispatcher * dispatcher;
		physx::PxControllerManager * controllerManager;

		std::vector<physx::PxActor *> actors;
		std::vector<physx::PxShape *> shapes;
		std::vector<physx::PxMaterial * > materials;

	public:

		void Simulate(float dt) {
			scene->simulate(dt);
			scene->fetchResults(true);
		}

		void AddActor(physx::PxActor * actor) {
			scene->addActor(*actor);
		}

		void CreateResources();

		void ReleaseResources();

		physx::PxPhysics * Get() const {
			return physics;
		}

	};

}
