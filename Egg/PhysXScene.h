#pragma once

#include "Modules.h"
#include <physx/PxPhysicsAPI.h>
#include <vector>

#define PX_RELEASE(ptr) if(ptr) { ptr->release(); ptr = nullptr; }

namespace Egg::Physics {

	//A wrapper class for the basic physx initialization and cleanup
	class PhysX {
	public:
		physx::PxDefaultAllocator allocator;
		physx::PxDefaultErrorCallback errorCallback;
		physx::PxFoundation * foundation;
		physx::PxPhysics * physics;
		physx::PxPvd * debugger;
		physx::PxDefaultCpuDispatcher * dispatcher;
		physx::PxCooking * cooking;

		void UpdateDebugCamera(const DirectX::XMFLOAT3 & pos, const DirectX::XMFLOAT3 & up, const DirectX::XMFLOAT3 & lookAt);

		void CreateResources();

		void ReleaseResources();


	};

}
