#pragma once

#include "../GameObject.h"
#include <Netcode/MathExt.h>

class PlayerScript : public ScriptBase {
public:
	std::string name;
	int kills;
	int deaths;
	int rtt;
	PlayerState state;
	float cameraPitch;
	float cameraYaw;

	PlayerScript() : name{}, kills{ 0 }, deaths{ 0 }, rtt{ -1 }, state{ PlayerState::SPECTATOR }, cameraPitch{}, cameraYaw{} {
		
	}
};

class GameScene;
class GameClient;

class LocalPlayerScript : public PlayerScript {
	Transform * transform;
	Transform * attachmentTransform;
	Camera * camera;
	Collider * collider;
	Netcode::PxPtr<physx::PxController> controller;
	Netcode::Float3 velocity;
	float mouseSpeed;
	Netcode::Float3 gravity;
	float avatarSpeed;
	Netcode::Duration reloadTimer;
	Netcode::Duration testAutoFireTimer;
	GameScene * scene;
	GameClient * client;
	bool leftClickHeld;
	bool debugMoveLeft;

	void UpdateLookDirection(float dt);

	Netcode::Vector3 GetDirectionalMovement() {
		float dx = Netcode::Input::GetAxis(AxisEnum::HORIZONTAL);
		float dz = Netcode::Input::GetAxis(AxisEnum::VERTICAL);

		Netcode::Vector3 dxdzVec = Netcode::Float3{ dx, 0.0f, dz };

		if(dxdzVec.AllZero()) {
			return dxdzVec;
		}

		return dxdzVec.Normalize();
	}

public:

	physx::PxController* GetController() {
		return controller.Get();
	}

	LocalPlayerScript(Netcode::PxPtr<physx::PxController> ctrl, GameClient * cli, GameObject * camObj, GameObject * attachmentNode) {
		camera = camObj->GetComponent<Camera>();
		attachmentTransform = attachmentNode->GetComponent<Transform>();
		controller = std::move(ctrl);
		cameraPitch = 0.6f;
		cameraYaw = 3.14f;
		mouseSpeed = 1.0f;
		avatarSpeed = 250.0f;
		gravity = Netcode::Float3{ 0.0f, -981.0f, 0.0f };
		velocity = Netcode::Float3{ 0.0f, 0.0f, 0.0f };
		client = cli;
	}

	virtual void BeginPlay(GameObject * owner) override;

	void HandleFireInput(Netcode::GameClock * gameClock);
	virtual void Update(Netcode::GameClock * gameClock) override;

	virtual void FixedUpdate(Netcode::GameClock * gameClock) override;
};
