#include "PhysXWrapper.h"
#include "Utility.h"
#include "MathExt.h"

namespace Netcode::Physics {
	PhysX::~PhysX()
	{
		ReleaseResources();
	}
	void PhysX::CreateResources() {
		foundation.Reset(PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback));

#if defined(NETCODE_DEBUG)
		debugger.Reset(PxCreatePvd(*foundation));
		const int port = 5425;
		physx::PxPvdTransport * transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", port, 10);
		bool isDebuggerConnected = debugger->connect(*transport, physx::PxPvdInstrumentationFlag::eDEBUG);

		if(!isDebuggerConnected) {
			Log::Warn("Failed to connect to PhysX Visual Debugger");
		} else {
			Log::Info("Connected to PVD:{0}", port);
		}
#endif

		physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, physx::PxTolerancesScale{ }, true, debugger.Get());
		dispatcher = physx::PxDefaultCpuDispatcherCreate(2);

		physx::PxTolerancesScale defaultToleranceScale;
		physx::PxCookingParams defaultCookingParams{ defaultToleranceScale };

		cooking = PxCreateCooking(PX_PHYSICS_VERSION, *foundation, defaultCookingParams);

		defaultMaterial = physics->createMaterial(0.5f, 0.5f, 0.5f);
	}

	void PhysX::ReleaseResources() {
		defaultMaterial.Reset();
		cooking.Reset();
		dispatcher.Reset();
		physics.Reset();

		if(debugger != nullptr) {
			PxPtr<physx::PxPvdTransport> transport = debugger->getTransport();
			debugger.Reset();
			transport.Reset();
		}

		foundation.Reset();
	}

}
