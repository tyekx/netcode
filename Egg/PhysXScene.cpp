#include "PhysXScene.h"
#include "Utility.h"
#include "EggMath.h"

namespace Egg::Physics {

	void PhysX::UpdateDebugCamera(const DirectX::XMFLOAT3 & pos, const DirectX::XMFLOAT3 & up, const DirectX::XMFLOAT3 & lookAt)
	{/*
		physx::PxPvdSceneClient * pvdClient = scene->getScenePvdClient();
		if(pvdClient) {
			pvdClient->updateCamera("default", ToPxVec3(pos), ToPxVec3(up), ToPxVec3(lookAt));
		}*/
	}

	void PhysX::CreateResources() {
		foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback);
		debugger = PxCreatePvd(*foundation);
		physx::PxPvdTransport * transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		bool isDebuggerConnected = debugger->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

		if(!isDebuggerConnected) {
			Egg::Utility::Debugf("Notice: Failed to connect to Physx Visual Debugger\r\n");
		} else {
			Egg::Utility::Debugf("PVD Connected on socket 5425\r\n");
		}

		physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, physx::PxTolerancesScale{ }, true, debugger);
		dispatcher = physx::PxDefaultCpuDispatcherCreate(2);

		physx::PxTolerancesScale defaultToleranceScale;
		physx::PxCookingParams defaultCookingParams{ defaultToleranceScale };

		cooking = PxCreateCooking(PX_PHYSICS_VERSION, *foundation, defaultCookingParams);
		/*
		physx::PxSceneDesc sceneDesc{ physics->getTolerancesScale() };
		sceneDesc.gravity = physx::PxVec3{ 0.0f, -981.0f, 0.0f };
		sceneDesc.cpuDispatcher = dispatcher;
		sceneDesc.filterShader = SimulationFilterShader;

		scene = physics->createScene(sceneDesc);
		controllerManager = PxCreateControllerManager(*scene);
		physx::PxPvdSceneClient * pvdClient = scene->getScenePvdClient();
		if(pvdClient) {
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}*/
	}

	void PhysX::ReleaseResources() {
		PX_RELEASE(cooking);
		PX_RELEASE(dispatcher);
		PX_RELEASE(physics);

		if(debugger) {
			physx::PxPvdTransport * transport = debugger->getTransport();
			debugger->release();
			debugger = nullptr;
			PX_RELEASE(transport);
		}
		PX_RELEASE(foundation);
	}

}
