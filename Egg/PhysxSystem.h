#pragma once

#include "PhysxSystem.h"
#include "GameObject.h"
#include <PxPhysicsAPI.h>

#define PX_RELEASE(ptr) if(ptr) { ptr->release(); ptr = nullptr; }

namespace Egg {

	class PhysxSystem {

	public:
		physx::PxDefaultAllocator allocator;
		physx::PxDefaultErrorCallback errorCallback;
		physx::PxFoundation * foundation;
		physx::PxPhysics * physics;
		physx::PxPvd * debugger;
		physx::PxScene * scene;
		physx::PxDefaultCpuDispatcher * dispatcher;
		physx::PxControllerManager * controllerManager;
		physx::PxRigidStatic * groundPlane;

		constexpr static SignatureType Required() {
			return (0x1ULL << TupleIndexOf<PhysxComponent, COMPONENTS_T>::value);
		}

		constexpr static SignatureType Incompatible() {
			return (0x0ULL);
		}

		PhysxSystem() : 
			allocator{},
			errorCallback{}, 
			foundation{ nullptr },
			physics{ nullptr },
			debugger{ nullptr },
			scene{ nullptr },
			dispatcher{ nullptr } { }

		~PhysxSystem() {
			ReleaseResources();
		}

		void ReleaseResources() {
			PX_RELEASE(physics);

			if(debugger) {
				physx::PxPvdTransport * transport = debugger->getTransport();
				debugger->release();
				debugger = nullptr;
				PX_RELEASE(transport);
			}
			PX_RELEASE(foundation);
		}

		void CreateResources() {
			foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback);
			debugger = PxCreatePvd(*foundation);
			physx::PxPvdTransport * transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
			bool isDebuggerConnected = debugger->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

			if(!isDebuggerConnected) {
				OutputDebugString("Notice: Failed to connect to Physx Visual Debugger\r\n");
			} else {
				OutputDebugString("PVD Connected on socket 5425\r\n");
			}
			physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, physx::PxTolerancesScale{}, true, debugger);
			dispatcher = physx::PxDefaultCpuDispatcherCreate(2);

			physx::PxSceneDesc sceneDesc{ physics->getTolerancesScale() };
			sceneDesc.gravity = physx::PxVec3{ 0.0f, -9.81f, 0.0f };
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
			groundPlane = physx::PxCreatePlane(*physics, physx::PxPlane{ 0, 1.0f, 0, 0 }, *groundMaterial);

			scene->addActor(*groundPlane);

		}

		void Simulate(float dt) {
			scene->simulate(dt);
			scene->fetchResults(true);
		}

		void Run(Egg::GameObject * gameObject) {
			PhysxComponent* pc = gameObject->GetComponent<PhysxComponent>();
		}

	};

}
