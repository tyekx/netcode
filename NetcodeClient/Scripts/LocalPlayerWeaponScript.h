#pragma once

#include "../GameObject.h"
#include <Netcode/MovementController.h>

class LocalPlayerWeaponScript : public ScriptBase {
	Transform * transform;
	Netcode::MovementController * movementController;
	float rot;
	//Netcode::Float3 target;
public:
	LocalPlayerWeaponScript(Transform * tr, Netcode::MovementController * ctrl) {
		transform = tr;
		movementController = ctrl;
		rot = 0.0f;
	}
	
	virtual void BeginPlay(GameObject * gameObject) override {

	}

	virtual void Update(float dt) override {

		//const Netcode::Vector3 mask = Netcode::Float3{ 0.0f, 0.1f, 0.5f };
		float scale = 0.33f;
		float timescale = 1.5f;
		Netcode::Vector3 mask = Netcode::Float3{ 0.0f, 1.0f, 0.2f };
		const Netcode::Vector3 unitZ = Netcode::Float3::UnitZ;

		if(movementController->IsMovingForward()) {
			mask = Netcode::Float3{ 0.0f, 0.2f, 1.0f };
			timescale = 10.0f;
			scale = 0.5f;
		}
		
		rot += timescale * dt;

		Netcode::Quaternion q{ rot, 0.0f, 0.0f };
		transform->position = unitZ.Rotate(q) * (mask * scale);
	}
};
