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

	DirectX::XMVECTOR cameraYawQuat = DirectX::XMQuaternionRotationRollPitchYaw(0.0f, cameraYaw, 0.0f);

	DirectX::XMFLOAT3A devCam = { devCamX, devCamY, devCamZ };
	DirectX::XMVECTOR devCamVec = DirectX::XMLoadFloat3A(&devCam);

	devCamVec = DirectX::XMVector3Rotate(devCamVec, cameraYawQuat);

	DirectX::XMVECTOR devCamPos = DirectX::XMLoadFloat3(&transform->position);
	devCamVec = DirectX::XMVectorScale(devCamVec, cameraSpeed * dt);
	devCamPos = DirectX::XMVectorAdd(devCamVec, devCamPos);
	DirectX::XMStoreFloat3(&transform->position, devCamPos);

	DirectX::XMFLOAT3 minusUnitZ{ 0.0f, 0.0f, -1.0f };
	DirectX::XMVECTOR cameraQuat = DirectX::XMQuaternionRotationRollPitchYaw(cameraPitch, cameraYaw, 0.0f);
	DirectX::XMVECTOR aheadStart = DirectX::XMLoadFloat3(&minusUnitZ);
	//DirectX::XMVECTOR camUp = DirectX::XMLoadFloat3(&camera->up);

	DirectX::XMStoreFloat3(&camera->ahead, DirectX::XMVector3Normalize(DirectX::XMVector3Rotate(aheadStart, cameraQuat)));

	//DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4A(.GetViewMatrix());
	//DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4A(&baseCam.GetProjMatrix());
	//DirectX::XMMATRIX vp = DirectX::XMMatrixMultiply(view, proj);
}
