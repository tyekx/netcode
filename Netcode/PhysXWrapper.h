#pragma once

#include "Modules.h"
#include "PxPtr.hpp"
#include <physx/PxPhysicsAPI.h>
#include <vector>
#include <memory>

#define PX_RELEASE(ptr) if(ptr) { ptr->release(); ptr = nullptr; }


namespace Netcode::Physics {

	//A wrapper class for the basic physx initialization and cleanup
	class PhysX {
	public:
		physx::PxDefaultAllocator allocator;
		physx::PxDefaultErrorCallback errorCallback;
		PxPtr<physx::PxFoundation> foundation;
		PxPtr<physx::PxPhysics> physics;
		PxPtr<physx::PxPvd> debugger;
		PxPtr<physx::PxDefaultCpuDispatcher> dispatcher;
		PxPtr<physx::PxCooking> cooking;

		void UpdateDebugCamera(const Float3 & pos, const Float3 & up, const Float3 & lookAt);

		void CreateResources();

		void ReleaseResources();


	};

}
