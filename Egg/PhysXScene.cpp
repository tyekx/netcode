#include "PhysXScene.h"
#include "Utility.h"
#include "EggMath.h"

namespace Egg::Physics {

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

		physx::PxSceneDesc sceneDesc{ physics->getTolerancesScale() };
		sceneDesc.gravity = physx::PxVec3{ 0.0f, -981.0f, 0.0f };
		sceneDesc.cpuDispatcher = dispatcher;
		sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

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
		for(auto * actor : actors) {
			scene->removeActor(*actor);
			PX_RELEASE(actor);
		}

		for(auto * shape : shapes) {
			PX_RELEASE(shape);
		}

		for(auto * material : materials) {
			PX_RELEASE(material);
		}

		PX_RELEASE(controllerManager);
		PX_RELEASE(scene);
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
