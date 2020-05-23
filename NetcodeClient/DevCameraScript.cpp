#include "DevCameraScript.h"
#include <algorithm>
#include <Windows.h>

void DevCameraScript::Setup(GameObject * gameObject) {
	Netcode::Input::SetAxis("DevCameraX", VK_NUMPAD6, VK_NUMPAD4);
	Netcode::Input::SetAxis("DevCameraZ", VK_NUMPAD5, VK_NUMPAD8);
	Netcode::Input::SetAxis("DevCameraY", VK_NUMPAD9, VK_NUMPAD7);
	transform = gameObject->GetComponent<Transform>();
	camera = gameObject->GetComponent<Camera>();
	cameraPitch = 0.0f;
	cameraYaw = 0.0f;
	cameraSpeed = 250.0f;
	mouseSpeed = 1.0f;
}

void DevCameraScript::Update(float dt) {
	float devCamX = Netcode::Input::GetAxis("DevCameraX");
	float devCamZ = Netcode::Input::GetAxis("DevCameraZ");
	float devCamY = Netcode::Input::GetAxis("DevCameraY");

	DirectX::XMINT2 mouseDelta = Netcode::Input::GetMouseDelta();

	DirectX::XMFLOAT2A normalizedMouseDelta{ -(float)(mouseDelta.x), -(float)(mouseDelta.y) };
	cameraPitch += mouseSpeed * normalizedMouseDelta.y * dt;
	cameraPitch = std::clamp(cameraPitch, -(DirectX::XM_PIDIV2 - 0.00001f), (DirectX::XM_PIDIV2 - 0.00001f));
	cameraYaw += mouseSpeed * normalizedMouseDelta.x * dt;

	if(cameraYaw < (-DirectX::XM_PI)) {
		cameraYaw += DirectX::XM_2PI;
	}

	if(cameraYaw > (DirectX::XM_PI)) {
		cameraYaw -= DirectX::XM_2PI;
	}

	Netcode::Quaternion cameraYawQuat{ 0.0f, cameraYaw, 0.0f };

	Netcode::Vector3 devCamVec = Netcode::Float3{ devCamX, devCamY, devCamZ };
	devCamVec = devCamVec.Rotate(cameraYawQuat);

	Netcode::Vector3 devCamPos = transform->position;
	transform->position = devCamPos + devCamVec * cameraSpeed * dt;

	Netcode::Vector3 aheadStart = Netcode::Float3{ 0.0f, 0.0f, -1.0f };
	Netcode::Quaternion cameraQuat = DirectX::XMQuaternionRotationRollPitchYaw(cameraPitch, cameraYaw, 0.0f);

	camera->ahead = aheadStart.Rotate(cameraQuat).Normalize();
}
