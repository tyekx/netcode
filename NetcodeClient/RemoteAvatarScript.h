#pragma once

#include "GameObject.h"

class RemoteAvatarScript : public IBehavior {
	Netcode::PxPtr<physx::PxController> controller;

public:
	RemoteAvatarScript(Netcode::PxPtr<physx::PxController> ctrl) : controller{ std::move(ctrl) } { }

	virtual void Setup(GameObject * gameObject) override;
	virtual void Update(float dt) override;
};
