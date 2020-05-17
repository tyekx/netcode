#pragma once

#include "GameObject.h"
#include <Netcode/GramSchmidt.h>

class PlayerBehavior : public IBehavior {
	Transform * transform;
	Camera * camera;
	Collider * collider;
	physx::PxController * controller;
	float cameraPitch;
	float cameraYaw;
	float mouseSpeed;
	float avatarSpeed;
	DirectX::XMFLOAT3 velocity;
	DirectX::XMVECTOR gravity;

	void UpdateLookDirection(float dt) {
		DirectX::XMINT2 mouseDelta = Netcode::Input::GetMouseDelta();

		DirectX::XMFLOAT2A normalizedMouseDelta{ -(float)(mouseDelta.x), -(float)(mouseDelta.y) };
		cameraPitch -= mouseSpeed * normalizedMouseDelta.y * dt;
		cameraPitch = std::clamp(cameraPitch, -(DirectX::XM_PIDIV2 - 0.0001f), (DirectX::XM_PIDIV2 - 0.0001f));
		cameraYaw += mouseSpeed * normalizedMouseDelta.x * dt;

		if(cameraYaw < (-DirectX::XM_PI)) {
			cameraYaw += DirectX::XM_2PI;
		}

		if(cameraYaw > (DirectX::XM_PI)) {
			cameraYaw -= DirectX::XM_2PI;
		}
	}

	DirectX::XMVECTOR GetDirectionalMovement() {
		float dx = Netcode::Input::GetAxis("Horizontal");
		float dz = Netcode::Input::GetAxis("Vertical");

		// 4th coord = 0 implies a vector value
		const DirectX::XMFLOAT3 dxdz{ dx, 0.0f, dz };
		DirectX::FXMVECTOR dxdzVec = DirectX::XMLoadFloat3(&dxdz);
		
		return DirectX::XMVector3Normalize(dxdzVec);
	}

public:

	PlayerBehavior(physx::PxController * ctrl, Camera * cam) {
		camera = cam;
		controller = ctrl;
		cameraPitch = 0.6f;
		cameraYaw = 3.14f;
		mouseSpeed = 1.0f;
		avatarSpeed = 250.0f;
		DirectX::XMFLOAT3 g{ 0.0f, -981.0f, 0.0f };
		gravity = DirectX::XMLoadFloat3(&g);
		velocity = DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };
	}

	virtual void Setup(GameObject * owner) override {
		transform = owner->GetComponent<Transform>();
		controller->setPosition(physx::PxExtendedVec3{ transform->position.x, transform->position.y, transform->position.z });
		collider = owner->GetComponent<Collider>();
	}

	virtual void Update(float dt) override {
		UpdateLookDirection(dt);

		DirectX::XMFLOAT3 minusUnitZ{ 0.0f, 0.0f, 1.0f };
		DirectX::XMVECTOR cameraQuat = DirectX::XMQuaternionRotationRollPitchYaw(cameraPitch, cameraYaw, 0.0f);
		DirectX::XMVECTOR aheadStart = DirectX::XMLoadFloat3(&minusUnitZ);
		DirectX::XMStoreFloat3(&camera->ahead, DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(aheadStart, cameraQuat)));

		DirectX::FXMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(0.0f, cameraYaw, 0.0f);
		DirectX::FXMVECTOR lDirectionalMovement = GetDirectionalMovement();
		DirectX::FXMVECTOR movementDeltaWorldSpace = DirectX::XMVector3Rotate(lDirectionalMovement, q);
		DirectX::XMVECTOR movementDeltaSpeedScaled = DirectX::XMVectorScale(movementDeltaWorldSpace, avatarSpeed * dt);

		DirectX::FXMVECTOR gravityDeltaVelocity = DirectX::XMVectorScale(gravity, dt);

		DirectX::XMVECTOR velocityVector = DirectX::XMLoadFloat3(&velocity);
		velocityVector = DirectX::XMVectorAdd(velocityVector, gravityDeltaVelocity);
		DirectX::XMStoreFloat3(&velocity, velocityVector);

		movementDeltaSpeedScaled = DirectX::XMVectorAdd(DirectX::XMVectorScale(velocityVector, dt), movementDeltaSpeedScaled);

		DirectX::XMFLOAT3 movementDelta;
		DirectX::XMStoreFloat3(&movementDelta, movementDeltaSpeedScaled);
		
		const physx::PxControllerCollisionFlags moveResult = controller->move(ToPxVec3(movementDelta), 0.0f, dt, physx::PxControllerFilters{});

		if(moveResult == physx::PxControllerCollisionFlag::eCOLLISION_DOWN) {
			velocity.y = 0.0f;
		}

		auto p = controller->getPosition();

		transform->position = DirectX::XMFLOAT3{
			static_cast<float>(p.x),
			static_cast<float>(p.y - 90.0),
			static_cast<float>(p.z)
		};

		DirectX::XMStoreFloat4(&transform->rotation, q);
		
		if(collider != nullptr) {
			if(auto * rigidBody = collider->actorRef->is<physx::PxRigidDynamic>()) {
				physx::PxTransform pxT;
				pxT.p = ToPxVec3(transform->position);
				pxT.q = ToPxQuat(transform->rotation);
				rigidBody->setKinematicTarget(pxT);
			}
		}
	}
};
