#pragma once

#include "GameObject.h"

class RemoteAvatarScript : public IBehavior {
	physx::PxController * controller;

public:
	RemoteAvatarScript(physx::PxController * ctrl) : controller{ ctrl } { }

	virtual void Setup(GameObject * gameObject) override;
	virtual void Update(float dt) override;
};
