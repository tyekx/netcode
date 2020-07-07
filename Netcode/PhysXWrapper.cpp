#include "PhysXWrapper.h"
#include "Utility.h"
#include "MathExt.h"

namespace Netcode::Physics {

	void PhysX::UpdateDebugCamera(const Float3 & pos, const Float3 & up, const Float3 & lookAt)
	{/*
		physx::PxPvdSceneClient * pvdClient = scene->getScenePvdClient();
		if(pvdClient) {
			pvdClient->updateCamera("default", ToPxVec3(pos), ToPxVec3(up), ToPxVec3(lookAt));
		}*/
	}

	void PhysX::CreateResources() {
		foundation.Reset(PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback));
		debugger.Reset(PxCreatePvd(*foundation));
		physx::PxPvdTransport * transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		bool isDebuggerConnected = debugger->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

		if(!isDebuggerConnected) {
			Netcode::Utility::Debugf("Notice: Failed to connect to Physx Visual Debugger\r\n");
		} else {
			Netcode::Utility::Debugf("PVD Connected on socket 5425\r\n");
		}

		physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, physx::PxTolerancesScale{ }, true, debugger.Get());
		dispatcher = physx::PxDefaultCpuDispatcherCreate(2);

		physx::PxTolerancesScale defaultToleranceScale;
		physx::PxCookingParams defaultCookingParams{ defaultToleranceScale };

		cooking = PxCreateCooking(PX_PHYSICS_VERSION, *foundation, defaultCookingParams);
	}

	void PhysX::ReleaseResources() {
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
