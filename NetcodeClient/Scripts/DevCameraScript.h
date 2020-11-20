#pragma once

#include "../GameObject.h"

class DevCameraScript : public ScriptBase {
public:
	Transform * transform;
	Camera * camera;
	float mouseSpeed;
	float cameraSpeed;
	float cameraPitch;
	float cameraYaw;


	virtual void BeginPlay(GameObject * gameObject) override;

	virtual void Update(Netcode::GameClock * clock) override;

};

class SocketScript : public ScriptBase {
public:
	Transform * transform;
	Animation * anim;
	Netcode::Float3 offset;
	uint32_t boneId;

	virtual void BeginPlay(GameObject* gameObject) override {
		transform = gameObject->GetComponent<Transform>();
	}

	virtual void Update(Netcode::GameClock * clock) override;
};
