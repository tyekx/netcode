#pragma once

#include "../GameObject.h"

class RemotePlayerScript : public ScriptBase {
	Netcode::PxPtr<physx::PxController> controller;

public:
	RemotePlayerScript(Netcode::PxPtr<physx::PxController> ctrl) : controller{ std::move(ctrl) } { }

	virtual void BeginPlay(GameObject * gameObject) override;
	virtual void Update(float dt) override;
};
