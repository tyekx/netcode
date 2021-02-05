#pragma once

#include "../GameObject.h"
#include <Netcode/Network/ReplicationContext.h>
#include "LocalPlayerScript.h"


struct ServerFrameData {
	Netcode::Float3 position;
	PlayerState state;
};

class RemotePlayerScript : public PlayerScript {
	Netcode::PxPtr<physx::PxController> controller;
	Transform* transform;
	Transform * rifleOriginTransform;
	Camera * camera;
	Network * network;
	Netcode::Duration interpolationDelay;
	Netcode::Float3 IND_positionOld;
	Netcode::Float3 IND_aheadOld;
	Netcode::Quaternion IND_yawQuatOld;
	Netcode::Quaternion IND_quatOld;
	Netcode::Quaternion IND_yawQuatCurrent;
	Netcode::Quaternion IND_quatCurrent;
public:
	Netcode::Float3 IND_position;
	Netcode::Float3 IND_ahead;
	
	Netcode::Timestamp serverLastUpdate;
	std::unique_ptr<HistoryBuffer<ServerFrameData>> serverHistory;

	physx::PxController* GetController() {
		return controller.Get();
	}
	
	RemotePlayerScript(Netcode::PxPtr<physx::PxController> ctrl, const Netcode::Duration & interpDelay) :
		controller{ std::move(ctrl) },
		transform{ nullptr },
		camera { nullptr },
		network { nullptr },
		interpolationDelay{ interpDelay } { }

	virtual void Construct(GameObject * gameObject) override;
	virtual void BeginPlay(GameObject * gameObject) override;
	virtual void Update(Netcode::GameClock* gameClock) override;
};
