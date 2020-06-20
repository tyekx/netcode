#pragma once

#include "GameObject.h"
#include <Netcode/Input.h>

class GunBehavior : public IBehavior {

public:
	Animation * animComponent;
	Transform * selfTransform;
	Transform * rootTransform;
	Netcode::Float4 localPosition;
	Netcode::Float4 localRotation;
	int socketId;
	float cameraPitch;

	GunBehavior(GameObject * avatarObject, GameObject * gunRootObj, const Netcode::Float4 & lp, const Netcode::Float4 & lq, int socket) {
		localPosition = lp;
		localRotation = lq;
		animComponent = avatarObject->GetComponent<Animation>();
		socketId = socket;
		cameraPitch = 0.6f;
		rootTransform = gunRootObj->GetComponent<Transform>();
	}

	virtual void Setup(GameObject * owner) override {
		selfTransform = owner->GetComponent<Transform>();
		selfTransform->rotation = localRotation;
		selfTransform->position = Netcode::Float3{ localPosition.x, localPosition.y, localPosition.z };
	}

	virtual void Update(float dt) override {
		Netcode::Int2 mouseDelta = Netcode::Input::GetMouseDelta();

		Netcode::Float2 normalizedMouseDelta{ -static_cast<float>(mouseDelta.x), -static_cast<float>(mouseDelta.y) };
		cameraPitch -= normalizedMouseDelta.y * dt;
		cameraPitch = std::clamp(cameraPitch, -(Netcode::C_PIDIV2 - 0.0001f), (Netcode::C_PIDIV2 - 0.0001f));

		Netcode::Quaternion pitchQuat{ cameraPitch, 0.0f, 0.0f };

		// x is (0, pi)
		float x = cameraPitch + Netcode::C_PIDIV2;
		float phi = 0.7f;

		float y = std::max(x - (Netcode::C_PI - phi), 0.0f);
		y += std::min(x - phi, 0.0f);

		Netcode::Quaternion headQuat{ cameraPitch, 0.0f, 0.0f };
		animComponent->headRotation = headQuat;
		Netcode::Quaternion gammaQuat{ y - Netcode::C_PIDIV2, 0.0f, 0.0f };
		rootTransform->rotation = pitchQuat;

		Netcode::Vector3 rootPos = rootTransform->position;
		Netcode::Vector3 pos = selfTransform->position;

		Netcode::Vector3 pitchPos = pos.Rotate(pitchQuat) + rootPos;
		pos = pos.Rotate(gammaQuat) + rootPos;

		animComponent->effectors[0].position = pos;
		animComponent->effectors[1].position = pitchPos;

			/*
		DirectX::XMMATRIX toRoot = DirectX::XMLoadFloat4x4A(&animComponent->debugBoneData->ToRootTransform[socketId]);
		DirectX::XMVECTOR lp = DirectX::XMLoadFloat4(&gunOffset);
		DirectX::XMFLOAT4 socketedPos;
		DirectX::XMStoreFloat4(&socketedPos, DirectX::XMVector4Transform(lp, DirectX::XMMatrixTranspose(toRoot)));
		*/

		//DirectX::XMStoreFloat4(&selfTransform->rotation, lq);
	}
};
