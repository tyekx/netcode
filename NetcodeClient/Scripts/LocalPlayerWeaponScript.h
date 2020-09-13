#pragma once

#include "../GameObject.h"
#include <Netcode/MovementController.h>

class LocalPlayerWeaponScript : public ScriptBase {
	Transform * transform;
	Netcode::MovementController * movementController;
	float t;
	Netcode::Float3 target;

	static Netcode::Vector3 GetDirectionalMovement() {
		float dx = Netcode::Input::GetAxis(AxisEnum::HORIZONTAL);
		float dz = Netcode::Input::GetAxis(AxisEnum::VERTICAL);

		Netcode::Vector3 dxdzVec = Netcode::Float3{ dx, 0.0f, dz };

		if(dxdzVec.AllZero()) {
			return dxdzVec;
		}

		return dxdzVec.Normalize();
	}
public:
	LocalPlayerWeaponScript(Transform * tr, Netcode::MovementController * ctrl) {
		transform = tr;
		movementController = ctrl;
		t = 0.0f;
	}
	
	virtual void BeginPlay(GameObject * gameObject) override {

	}

	virtual void Update(float dt) override {

		float scale = 0.33f;
		Netcode::Vector3 mask = Netcode::Float3{ 0.0f, 1.0f, 0.2f };
		if(movementController->IsIdle()) {
			t += 1.5f * dt;
		} else {
			mask = GetDirectionalMovement() + Netcode::Float3{ 0.0f, 0.2f, 0.0f };
			scale = 1.0f;
			t += 10.0f * dt;
		}

		const Netcode::Vector3 unitZ = Netcode::Float3::UnitZ;

		const Netcode::Quaternion q{ t, 0.0f, 0.0f };
		transform->position = unitZ.Rotate(q) * (mask * scale);
	}
};
