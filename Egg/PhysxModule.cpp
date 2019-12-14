#include "PhysxModule.h"
#include "Utility.h"
#include "EggMath.h"

namespace Egg::Physics {

	void PhysXModule::Start(Module::AApp * app) {
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

	void PhysXModule::Shutdown() {
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

	void * PhysXModule::GetUnderlyingPointer() {
		return nullptr;
	}

	HPXMAT PhysXModule::CreateMaterial(float staticFriction, float dynamicFriction, float restitution) {
		auto * ptr = physics->createMaterial(staticFriction, dynamicFriction, restitution);
		materials.push_back(ptr);
		return ptr;
	}

	HACTOR PhysXModule::CreateStaticActor(HSHAPE firstShape) {
		auto * actor = physx::PxCreateStatic(*physics, physx::PxTransform{ physx::PxIdentity }, *(reinterpret_cast<physx::PxShape *>(firstShape)));
		actors.push_back(actor);
		return actor;
	}

	HACTOR PhysXModule::CreateDynamicActor(HSHAPE firstShape, float density) {
		auto * actor = physx::PxCreateDynamic(*physics, physx::PxTransform{ physx::PxIdentity }, *(reinterpret_cast<physx::PxShape *>(firstShape)), density);
		actors.push_back(actor);
		return actor;
	}

	HACTOR PhysXModule::CreateKinematicActor(HSHAPE firstShape, float density) {
		auto * actor = physx::PxCreateKinematic(*physics, physx::PxTransform{ physx::PxIdentity }, *(reinterpret_cast<physx::PxShape *>(firstShape)), density);
		actors.push_back(actor);
		return actor;
	}

	HSHAPE PhysXModule::CreateBox(HPXMAT material, const DirectX::XMFLOAT3 & halfExtensions) {
		physx::PxBoxGeometry geometry{ ToPxVec3(halfExtensions) };

		auto * shape = physics->createShape(geometry, *(reinterpret_cast<physx::PxMaterial *>(material)));
		shapes.push_back(shape);
		return shape;
	}

	HACTOR PhysXModule::CreatePlane(HPXMAT material, const DirectX::XMFLOAT3 & normalVector, float distanceFromOrigin) {
		physx::PxPlaneGeometry geometry;

		physx::PxPlane plane{ ToPxVec3(normalVector) , distanceFromOrigin };

		auto * actor = physx::PxCreatePlane(*physics, plane, *(reinterpret_cast<physx::PxMaterial *>(material)));;
		actors.push_back(actor);
		return actor;
	}


	void PhysXModule::AddToScene(HACTOR actor) {
		scene->addActor(*(static_cast<physx::PxActor *>(actor)));
	}

	void PhysXModule::RemoveFromScene(HACTOR actor) {
		scene->removeActor(*(static_cast<physx::PxActor *>(actor)));
	}


	void PhysXModule::SetShapeLocalPosition(HSHAPE shape, const DirectX::XMFLOAT3 & position) {
		physx::PxShape * sp = reinterpret_cast<physx::PxShape *>(shape);
		physx::PxTransform transform = sp->getLocalPose();
		transform.p = ToPxVec3(position);
		sp->setLocalPose(transform);
	}

	void PhysXModule::SetShapeLocalQuaternion(HSHAPE shape, const DirectX::XMFLOAT4 & quaternion) {
		physx::PxShape * sp = reinterpret_cast<physx::PxShape *>(shape);
		physx::PxTransform transform = sp->getLocalPose();
		transform.q = ToPxQuat(quaternion);
		sp->setLocalPose(transform);
	}

	void PhysXModule::SetGravity(const DirectX::XMFLOAT3 & gravityVector) {
		scene->setGravity(ToPxVec3(gravityVector));
	}

	void PhysXModule::SetActorPosition(HACTOR actor, const DirectX::XMFLOAT3 & position) {
		physx::PxRigidBody * pxRigidBody = static_cast<physx::PxRigidBody *>(actor);

		physx::PxTransform transform = pxRigidBody->getGlobalPose();
		transform.p = ToPxVec3(position);
		pxRigidBody->setGlobalPose(transform);
	}

	void PhysXModule::SetActorRotation(HACTOR actor, const DirectX::XMFLOAT4 & quaternion) {
		physx::PxRigidBody * pxRigidBody = static_cast<physx::PxRigidBody *>(actor);

		physx::PxTransform transform = pxRigidBody->getGlobalPose();
		transform.q = ToPxQuat(quaternion);
		pxRigidBody->setGlobalPose(transform);
	}

	DirectX::XMFLOAT3 PhysXModule::GetActorPosition(HACTOR actor) {
		physx::PxRigidBody * pxRigidBody = static_cast<physx::PxRigidBody *>(actor);

		return ToFloat3(pxRigidBody->getGlobalPose().p);
	}

	DirectX::XMFLOAT4 PhysXModule::GetActorRotation(HACTOR actor) {
		physx::PxRigidBody * pxRigidBody = static_cast<physx::PxRigidBody *>(actor);

		return ToFloat4(pxRigidBody->getGlobalPose().q);
	}

	void PhysXModule::Simulate(float dt) {
		scene->simulate(dt);
		scene->fetchResults(true);
	}

}
