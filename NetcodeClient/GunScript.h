#pragma once

#include "GameObject.h"
#include <Netcode/Input.h>

class GunBehavior : public IBehavior {

public:
	Animation * animComponent;
	Transform * selfTransform;
	Transform * rootTransform;
	DirectX::XMFLOAT4 localPosition;
	DirectX::XMFLOAT4 localRotation;
	int socketId;
	float cameraPitch;

	GunBehavior(GameObject * avatarObject, GameObject * gunRootObj, const DirectX::XMFLOAT4 & lp, const DirectX::XMFLOAT4 & lq, int socket) {
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
		selfTransform->position = DirectX::XMFLOAT3{ localPosition.x, localPosition.y, localPosition.z };
	}

	virtual void Update(float dt) override {
		DirectX::XMINT2 mouseDelta = Netcode::Input::GetMouseDelta();

		DirectX::XMFLOAT2A normalizedMouseDelta{ -(float)(mouseDelta.x), -(float)(mouseDelta.y) };
		cameraPitch -= normalizedMouseDelta.y * dt;
		cameraPitch = std::clamp(cameraPitch, -(DirectX::XM_PIDIV2 - 0.0001f), (DirectX::XM_PIDIV2 - 0.0001f));

		DirectX::XMVECTOR pitchQuat = DirectX::XMQuaternionRotationRollPitchYaw(cameraPitch, 0.0f, 0.0f);

		// x is (0, pi)
		float x = cameraPitch + DirectX::XM_PIDIV2;
		float phi = 0.7f;

		float y = std::max(x - (DirectX::XM_PI - phi), 0.0f);
		y += std::min(x - phi, 0.0f);

		DirectX::XMVECTOR headQuat = DirectX::XMQuaternionRotationRollPitchYaw(cameraPitch, 0.0f, 0.0f);
		animComponent->headRotation = headQuat;

		DirectX::XMVECTOR gammaQuat = DirectX::XMQuaternionRotationRollPitchYaw(y - DirectX::XM_PIDIV2, 0.0f, 0.0f);

		DirectX::XMStoreFloat4(&rootTransform->rotation, pitchQuat);

		DirectX::XMVECTOR rootPos = DirectX::XMLoadFloat3(&rootTransform->position);
		DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&selfTransform->position);
		DirectX::XMVECTOR pitchPos = DirectX::XMVector3Rotate(DirectX::XMVectorScale(pos, 1.0f), pitchQuat);
		pitchPos = DirectX::XMVectorAdd(pitchPos, rootPos);
		pos = DirectX::XMVector3Rotate(pos, gammaQuat);
		pos = DirectX::XMVectorAdd(pos, rootPos);

		DirectX::XMStoreFloat4(&animComponent->effectors[0].position, pos);
		animComponent->effectors[0].position.w = 1.0f;
		
		DirectX::XMStoreFloat4(&animComponent->effectors[1].position, pitchPos);
		animComponent->effectors[1].position.w = 1.0f;

			/*
		DirectX::XMMATRIX toRoot = DirectX::XMLoadFloat4x4A(&animComponent->debugBoneData->ToRootTransform[socketId]);
		DirectX::XMVECTOR lp = DirectX::XMLoadFloat4(&gunOffset);
		DirectX::XMFLOAT4 socketedPos;
		DirectX::XMStoreFloat4(&socketedPos, DirectX::XMVector4Transform(lp, DirectX::XMMatrixTranspose(toRoot)));
		*/

		//DirectX::XMStoreFloat4(&selfTransform->rotation, lq);
	}
};
