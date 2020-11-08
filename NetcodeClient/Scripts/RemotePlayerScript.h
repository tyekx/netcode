#pragma once

#include "../GameObject.h"
#include <Netcode/Network/ReplicationContext.h>

class RemotePlayerScript : public ScriptBase {
	Netcode::PxPtr<physx::PxController> controller;

public:
	RemotePlayerScript(Netcode::PxPtr<physx::PxController> ctrl) : controller{ std::move(ctrl) } { }

	virtual void ReplicateReceive(GameObject* obj, nn::ReplicationContext* ctx) override {
		// IND buffer here?
	}
	
	virtual void BeginPlay(GameObject * gameObject) override;
	virtual void Update(float dt) override;
};
