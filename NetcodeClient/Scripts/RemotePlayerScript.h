#pragma once

#include "../GameObject.h"
#include <Netcode/Network/ReplicationContext.h>
#include "LocalPlayerScript.h"

class RemotePlayerScript : public PlayerScript {
	Netcode::PxPtr<physx::PxController> controller;
	Transform* transform;
	Camera * camera;
	Network * network;
	Netcode::Float3 IND_positionOld;
	Netcode::Float3 IND_aheadOld;
	Netcode::Quaternion IND_yawQuatOld;
	Netcode::Quaternion IND_quatOld;
	Netcode::Quaternion IND_yawQuatCurrent;
	Netcode::Quaternion IND_quatCurrent;
public:
	Netcode::Float3 IND_position;
	Netcode::Float3 IND_ahead;
	
	RemotePlayerScript(Netcode::PxPtr<physx::PxController> ctrl) : controller{ std::move(ctrl) } { }

	virtual void Construct(GameObject * gameObject) override;
	virtual void BeginPlay(GameObject * gameObject) override;
	virtual void Update(Netcode::GameClock* gameClock) override;
};
