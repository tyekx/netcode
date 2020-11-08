#pragma once

#include "../GameObject.h"
#include <Netcode/MathExt.h>

class PlayerScript : public ScriptBase {
protected:
	std::string name;
	int kills;
	int deaths;
	int rtt;
	Netcode::Float3 position; // IND buffered
	Netcode::Float3 lookAt;
};

class LocalPlayerScript : public ScriptBase {
	Transform * transform;
	Transform * attachmentTransform;
	Camera * camera;
	Collider * collider;
	Netcode::PxPtr<physx::PxController> controller;
	float cameraPitch;
	float cameraYaw;
	Netcode::Float3 velocity;
	float mouseSpeed;
	Netcode::Float3 gravity;
	float avatarSpeed;
	ClientActionBuffer actionBuffer;

	void UpdateLookDirection(float dt) {
		Netcode::Int2 mouseDelta = Netcode::Input::GetMouseDelta();

		Netcode::Float2 normalizedMouseDelta{ -(float)(mouseDelta.x), -(float)(mouseDelta.y) };
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

	Netcode::Vector3 GetDirectionalMovement() {
		float dx = Netcode::Input::GetAxis(AxisEnum::HORIZONTAL);
		float dz = Netcode::Input::GetAxis(AxisEnum::VERTICAL);

		Netcode::Vector3 dxdzVec = Netcode::Float3{ dx, 0.0f, dz };

		if(dxdzVec.AllZero()) {
			return dxdzVec;
		}

		return dxdzVec.Normalize();
	}

public:

	LocalPlayerScript(Netcode::PxPtr<physx::PxController> ctrl, GameObject * camObj, GameObject * attachmentNode) {
		camera = camObj->GetComponent<Camera>();
		attachmentTransform = attachmentNode->GetComponent<Transform>();
		controller = std::move(ctrl);
		cameraPitch = 0.6f;
		cameraYaw = 3.14f;
		mouseSpeed = 1.0f;
		avatarSpeed = 250.0f;
		gravity = Netcode::Float3{ 0.0f, -981.0f, 0.0f };
		velocity = Netcode::Float3{ 0.0f, 0.0f, 0.0f };
	}

	virtual void BeginPlay(GameObject * owner) override {
		transform = owner->GetComponent<Transform>();
		controller->setPosition(physx::PxExtendedVec3{ transform->position.x, transform->position.y, transform->position.z });
		collider = owner->GetComponent<Collider>();
	}

	virtual void Update(float dt) override {
		UpdateLookDirection(dt);

		Netcode::Quaternion cameraQuat{ cameraPitch, cameraYaw, 0.0f };
		Netcode::Vector3 aheadStart = Netcode::Float3{ 0.0f, 0.0f, 1.0f };
		camera->ahead = aheadStart.Rotate(cameraQuat).Normalize();

		Netcode::Quaternion cameraYawQuat{ 0.0f, cameraYaw, 0.0f };
		Netcode::Vector3 lDirectionalMovement = GetDirectionalMovement();
		Netcode::Vector3 movementDeltaWorldSpace = lDirectionalMovement.Rotate(cameraYawQuat);
		Netcode::Vector3 movementDeltaSpeedScaled = movementDeltaWorldSpace * avatarSpeed * dt;

		Netcode::Vector3 gravityDeltaVelocity = Netcode::Vector3{ gravity } *dt;

		Netcode::Vector3 velocityVector = velocity;

		velocity = velocityVector + gravityDeltaVelocity;

		const Netcode::Vector3 movementDelta = movementDeltaSpeedScaled + velocityVector * dt;

		const physx::PxControllerCollisionFlags moveResult = controller->move(ToPxVec3(movementDelta), 0.0f, dt, physx::PxControllerFilters{});

		if(moveResult == physx::PxControllerCollisionFlag::eCOLLISION_DOWN) {
			velocity.y = 0.0f;
		}

		auto footPos = controller->getFootPosition();

		transform->position = Netcode::Float3{
			static_cast<float>(footPos.x),
			static_cast<float>(footPos.y),
			static_cast<float>(footPos.z)
		};

		if(!movementDelta.AllZero()) {
			ClientPredictedMovement cm;
			cm.position = transform->position;
			cm.delta = velocity;
			actionBuffer.Add(ClientAction::Move(cm));
		}

		transform->rotation = cameraYawQuat;
		attachmentTransform->rotation = Netcode::Quaternion{ cameraPitch, 0.0f, 0.0f };
	}
};
