#include "PhysXScene.h"
#include "Utility.h"
#include "EggMath.h"

namespace Egg::Physics {

	void PhysXScene::UpdateDebugCamera(const DirectX::XMFLOAT3 & pos, const DirectX::XMFLOAT3 & up, const DirectX::XMFLOAT3 & lookAt)
	{/*
		physx::PxPvdSceneClient * pvdClient = scene->getScenePvdClient();
		if(pvdClient) {
			pvdClient->updateCamera("default", ToPxVec3(pos), ToPxVec3(up), ToPxVec3(lookAt));
		}*/
	}

	/*
	dont use any global memory
	*/
	static physx::PxFilterFlags SimulationFilterShader(
		physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
		physx::PxPairFlags & pairFlags, const void * constantBlock, physx::PxU32 constantBlockSize)
	{
		// let triggers through
		if(physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;

			if(filterData0.word0 == 1 || filterData1.word0 == 1) {
				return physx::PxFilterFlag::eSUPPRESS;
			}

			return physx::PxFilterFlag::eDEFAULT;
		}

		pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

		// trigger the contact callback for pairs (A,B) where
		// the filtermask of A contains the ID of B and vice versa.
		if((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;

		return physx::PxFilterFlag::eDEFAULT;
	}

	void PhysXScene::CreateResources() {
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
		}
	}

	void PhysXScene::ReleaseResources() {
		PX_RELEASE(controllerManager);
		PX_RELEASE(scene);
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
