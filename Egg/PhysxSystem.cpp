#include "PhysxSystem.h"

namespace Egg {

	PhysxSystem::PhysxSystem() :
		allocator{},
		errorCallback{},
		foundation{ nullptr },
		physics{ nullptr },
		debugger{ nullptr },
		scene{ nullptr },
		dispatcher{ nullptr },
		controllerManager{ nullptr },
		groundPlane{ nullptr } { }

	PhysxSystem::~PhysxSystem() {
		ReleaseResources();
	}

	void PhysxSystem::ReleaseResources() {
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

	void PhysxSystem::CreateResources() {
		foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback);
		debugger = PxCreatePvd(*foundation);
		physx::PxPvdTransport * transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		bool isDebuggerConnected = debugger->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

		if(!isDebuggerConnected) {
			OutputDebugString("Notice: Failed to connect to Physx Visual Debugger\r\n");
		} else {
			OutputDebugString("PVD Connected on socket 5425\r\n");
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
		physx::PxMaterial * groundMaterial = physics->createMaterial(0.5f, 0.5f, 0.6f);
		groundPlane = physx::PxCreatePlane(*physics, physx::PxPlane{ 0.0f, 1.0f, 0.0f, 0 }, *groundMaterial);
		physx::PxTransform planeTransform;
		//planeTransform.q = physx::PxQuat{ physx::PxIdentity };
		//planeTransform.p = physx::PxVec3{ 0.0f, 0.0f, 0.0f };
		//groundPlane->setGlobalPose(planeTransform);

		scene->addActor(*groundPlane);
	}

	void PhysxSystem::Simulate(float dt) {
		scene->simulate(dt);
		scene->fetchResults(true);
	}

	void PhysxSystem::Run(Egg::GameObject * gameObject) {
		PhysxComponent * pc = gameObject->GetComponent<PhysxComponent>();
	}

}
