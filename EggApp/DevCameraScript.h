#pragma once

#include "GameObject.h"

class DevCameraScript : public IBehavior {
	Transform * transform;
	Camera * camera;
	float mouseSpeed;
	float cameraSpeed;
	float cameraPitch;
	float cameraYaw;
public:


	// Inherited via IBehavior
	virtual void Setup(GameObject * gameObject) override;

	virtual void Update(float dt) override;

};
