#include "DevCameraScript.h"
#include <algorithm>

void DevCameraScript::BeginPlay(GameObject * gameObject) {
	transform = gameObject->GetComponent<Transform>();
	camera = gameObject->GetComponent<Camera>();
	cameraPitch = 0.0f;
	cameraYaw = 0.0f;
	cameraSpeed = 250.0f;
	camera->up = Netcode::Float3::UnitY;
	camera->ahead = Netcode::Float3::UnitZ;
	camera->nearPlane = 1.0f;
	camera->farPlane = 10000.0f;
	mouseSpeed = 1.0f;

	cameraYaw = Netcode::C_PI;
}

void DevCameraScript::Update(Netcode::GameClock* clock) {
	const float dt = clock->FGetDeltaTime();
	
	float devCamX = Netcode::Input::GetAxis(AxisEnum::DEV_CAM_X);
	float devCamZ = Netcode::Input::GetAxis(AxisEnum::DEV_CAM_Y);
	float devCamY = Netcode::Input::GetAxis(AxisEnum::DEV_CAM_Z);

	Netcode::Int2 mouseDelta = Netcode::Int2::Zero; //Netcode::Input::GetMouseDelta();

	Netcode::Float2 normalizedMouseDelta{ -(float)(mouseDelta.x), (float)(mouseDelta.y) };
	cameraPitch -= mouseSpeed * normalizedMouseDelta.y * dt;
	cameraPitch = std::clamp(cameraPitch, -(DirectX::XM_PIDIV2 - 0.0001f), (DirectX::XM_PIDIV2 - 0.0001f));

	cameraYaw += mouseSpeed * normalizedMouseDelta.x * dt;

	if(cameraYaw < (-DirectX::XM_PI)) {
		cameraYaw += DirectX::XM_2PI;
	}

	if(cameraYaw > (DirectX::XM_PI)) {
		cameraYaw -= DirectX::XM_2PI;
	}

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

void SocketScript::Update(Netcode::GameClock * clock) {
	if(anim->debugBoneData != nullptr) {
		const Netcode::Matrix bindTransform =
			Netcode::Matrix{ anim->debugBoneData->ToRootTransform[boneId] }.Transpose();

		
		Netcode::Vector4 offsetVec =
			Netcode::Vector3{ offset }.Extend(1.0f);

		offsetVec = offsetVec.Transform(bindTransform);
		offsetVec /= offsetVec.Swizzle<3, 3, 3, 3>();

		transform->position = offsetVec.Swizzle<0, 1, 2>();
	}
}
