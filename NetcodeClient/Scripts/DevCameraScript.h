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

	virtual void Update(float dt) override;

};
