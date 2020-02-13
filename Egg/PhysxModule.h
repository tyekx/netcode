#pragma once

#include "Modules.h"
#include <physx/PxPhysicsAPI.h>
#include <vector>

#define PX_RELEASE(ptr) if(ptr) { ptr->release(); ptr = nullptr; }

namespace Egg::Physics {

	class PhysXModule : public Module::IPhysicsModule {
		physx::PxDefaultAllocator allocator;
		physx::PxDefaultErrorCallback errorCallback;
		physx::PxFoundation * foundation;
		physx::PxPhysics * physics;
		physx::PxPvd * debugger;
		physx::PxScene * scene;
		physx::PxDefaultCpuDispatcher * dispatcher;
		physx::PxControllerManager * controllerManager;

		std::vector<physx::PxActor *> actors;
		std::vector<physx::PxShape *> shapes;
		std::vector<physx::PxMaterial * > materials;

	public:
		virtual void Start(Module::AApp * app) override;

		virtual void Shutdown() override;

		virtual void * GetUnderlyingPointer() override;

		virtual void SetGravity(const DirectX::XMFLOAT3 & gravityVector) override;

		virtual HPXMAT CreateMaterial(float staticFriction, float dynamicFriction, float restitution) override;

		virtual HACTOR CreateStaticActor(HSHAPE firstShape) override;

		virtual HACTOR CreateDynamicActor(HSHAPE firstShape, float density) override;

		virtual HACTOR CreateKinematicActor(HSHAPE firstShape, float density) override;

		virtual HSHAPE CreateBox(HPXMAT material, const DirectX::XMFLOAT3 & halfExtensions) override;

		virtual HSHAPE CreatePlane(HPXMAT material, const DirectX::XMFLOAT3 & normalVector, float distanceFromOrigin) override;

		virtual HSHAPE CreateCapsule(HPXMAT material, const DirectX::XMFLOAT2 & capsuleArgs) override;

		virtual HSHAPE CreateSphere(HPXMAT material, float radius) override;

		virtual void AttachShape(HACTOR actor, HSHAPE shape) override;

		virtual void AddToScene(HACTOR actor) override;

		virtual void RemoveFromScene(HACTOR actor) override;

		virtual void SetShapeLocalPosition(HSHAPE shape, const DirectX::XMFLOAT3 & position) override;

		virtual void SetShapeLocalQuaternion(HSHAPE shape, const DirectX::XMFLOAT4 & quaternion) override;

		virtual void SetActorPosition(HACTOR actor, const DirectX::XMFLOAT3 & position) override;

		virtual void SetActorRotation(HACTOR actor, const DirectX::XMFLOAT4 & quaternion) override;

		virtual DirectX::XMFLOAT3 GetActorPosition(HACTOR actor) override;

		virtual DirectX::XMFLOAT4 GetActorRotation(HACTOR actor) override;

		virtual void Simulate(float dt) override;

	};

}
