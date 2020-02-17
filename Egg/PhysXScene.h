#pragma once

#include "Modules.h"
#include <physx/PxPhysicsAPI.h>
#include <vector>

#define PX_RELEASE(ptr) if(ptr) { ptr->release(); ptr = nullptr; }

namespace Egg::Physics {

	class PhysXScene {
	public:
		physx::PxDefaultAllocator allocator;
		physx::PxDefaultErrorCallback errorCallback;
		physx::PxFoundation * foundation;
		physx::PxPhysics * physics;
		physx::PxPvd * debugger;
		physx::PxScene * scene;
		physx::PxDefaultCpuDispatcher * dispatcher;
		physx::PxControllerManager * controllerManager;
		physx::PxCooking * cooking;

		void Simulate(float dt) {
			scene->simulate(dt);
			scene->fetchResults(true);
		}

		void AddActor(physx::PxActor * actor) {
			scene->addActor(*actor);
		}

		void UpdateDebugCamera(const DirectX::XMFLOAT3 & pos, const DirectX::XMFLOAT3 & up, const DirectX::XMFLOAT3 & lookAt);

		void CreateResources();

		void ReleaseResources();

		physx::PxPhysics * Get() const {
			return physics;
		}

		physx::PxController * CreateController() {
			

			physx::PxCapsuleControllerDesc cd;
			cd.behaviorCallback = NULL;
			cd.climbingMode = physx::PxCapsuleClimbingMode::eEASY;
			cd.contactOffset = 0.1f;
			cd.density = 10.0f;
			cd.invisibleWallHeight = 0.0f;
			cd.material = physics->createMaterial(0.5f, 0.6f, 0.6f);
			cd.position = physx::PxExtendedVec3{ 0.0f, 200.0f, 0.0f };
			cd.nonWalkableMode = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING;
			cd.registerDeletionListener = true;
			cd.reportCallback = NULL;
			cd.scaleCoeff = 0.8f;
			cd.slopeLimit = 0.7071f;
			cd.stepOffset = 5.0f;
			cd.upDirection = physx::PxVec3{ 0.0f, 1.0f, 0.0f };
			cd.volumeGrowth = 1.5f;
			cd.height = 80.0f;
			cd.radius = 60.0f;
			/*
			physx::PxCapsuleControllerDesc cc;
			cc.climbingMode = physx::PxCapsuleClimbingMode::eEASY;
			cc.behaviorCallback = nullptr;
			cc.contactOffset = 0.1f;
			cc.density = 10.0f;
			cc.height = 80.0f;
			cc.invisibleWallHeight = 0.0f;
			cc.material = physics->createMaterial(0.5f, 0.6f, 0.6f);
			cc.maxJumpHeight = 1.0f;
			cc.nonWalkableMode = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING;
			cc.position = physx::PxExtendedVec3(0.0f, 200.0f, 0.0f);
			cc.radius = 60.0f;
			cc.registerDeletionListener = true;
			cc.reportCallback = nullptr;
			cc.scaleCoeff = 0.8f;
			cc.slopeLimit = 0.707f;
			cc.stepOffset = 5.0f;
			cc.upDirection = physx::PxVec3{ 0.0f, 1.0f, 0.0f };
			cc.volumeGrowth = 1.5f;
			cc.userData = nullptr;*/
			return controllerManager->createController(cd);
		}

	};

}
