#pragma once

#include "GameObject.h"

class GunBehavior : public IBehavior {

public:
	Animation * animComponent;
	Transform * selfTransform;
	DirectX::XMFLOAT4 localPosition;
	DirectX::XMFLOAT4 localRotation;
	int socketId;

	GunBehavior(GameObject * avatarObject, const DirectX::XMFLOAT4 & lp, const DirectX::XMFLOAT4 & lq, int socket) {
		localPosition = lp;
		localRotation = lq;
		animComponent = avatarObject->GetComponent<Animation>();
		socketId = socket;
	}

	virtual void Setup(GameObject * owner) override {
		selfTransform = owner->GetComponent<Transform>();
	}

	virtual void Update(float dt) override {
		DirectX::XMMATRIX toRoot = DirectX::XMLoadFloat4x4A(&animComponent->debugBoneData->ToRootTransform[socketId]);
		DirectX::XMVECTOR lp = DirectX::XMLoadFloat4(&localPosition);
		DirectX::XMVECTOR lq = DirectX::XMLoadFloat4(&localRotation);
		DirectX::XMFLOAT4 socketedPos;
		DirectX::XMStoreFloat4(&socketedPos, DirectX::XMVector4Transform(lp, DirectX::XMMatrixTranspose(toRoot)));

		selfTransform->position = DirectX::XMFLOAT3{
			socketedPos.x / socketedPos.w,
			socketedPos.y / socketedPos.w,
			socketedPos.z / socketedPos.w,
		};

		DirectX::XMStoreFloat4(&selfTransform->rotation, lq);
	}
};
