#include "RemotePlayerScript.h"
#include "../Network/NetwUtil.h"

void RemotePlayerScript::Construct(GameObject * gameObject) {
	transform = gameObject->GetComponent<Transform>();
	camera = gameObject->GetComponent<Camera>();
	network = gameObject->GetComponent<Network>();
}

void RemotePlayerScript::BeginPlay(GameObject * gameObject) {
	rifleOriginTransform = gameObject->Children()[0]->GetComponent<Transform>();

	rifleOriginTransform->position =
		Netcode::Vector3{ Netcode::Float3{ 0.0f, 160.0f, 0.0f } } + 
		Netcode::Float3{ -36.0f, -35.0f, 3.8f };
}

static Netcode::Quaternion GetYawQuatFromAhead(const Netcode::Float3& ahead) {
	const float yaw = atan2(ahead.x, ahead.z);
	return Netcode::Quaternion{ 0.0f, yaw, 0.0f };
}

static Netcode::Quaternion GetQuatFromAhead(const Netcode::Float3 & ahead) {
	const float yaw = atan2(std::clamp(ahead.x, -0.995f, 0.995f), std::clamp(ahead.z, -0.995f, 0.995f));
	const float pitch = -asinf(std::clamp(ahead.y, -0.995f, 0.995f));
	return Netcode::Quaternion{ pitch, yaw, 0.0f };
}

void RemotePlayerScript::Update(Netcode::GameClock * clock) {
	Netcode::Timestamp currentTime = clock->GetLocalTime();

	if(currentTime == network->updatedAt) {
		IND_positionOld = transform->position;
		IND_aheadOld = camera->ahead;
		IND_yawQuatOld = GetYawQuatFromAhead(camera->ahead);
		IND_quatOld = GetQuatFromAhead(camera->ahead);
		IND_yawQuatCurrent = GetYawQuatFromAhead(IND_ahead);
		IND_quatCurrent = GetQuatFromAhead(IND_ahead);
	} else {
		Netcode::Duration diff = currentTime - network->updatedAt;

		float t;
		if(diff < std::chrono::seconds(0)) {
			t = 0.0f;
		} else if(diff > interpolationDelay) {
			t = 1.0f;
		} else {
			t = std::chrono::duration<float>(diff).count() / std::chrono::duration<float>(interpolationDelay).count();
		}

		transform->position = Netcode::Vector3::Lerp(IND_positionOld, IND_position, t);
		const Netcode::Quaternion q0 = Netcode::Quaternion::Slerp(IND_yawQuatOld, IND_yawQuatCurrent, t);
		transform->rotation = q0;
		
		const Netcode::Quaternion q1 = Netcode::Quaternion::Slerp(IND_quatOld, IND_quatCurrent, t);

		const Netcode::Vector3 aheadV = Netcode::Vector3{ Netcode::Float3::UnitZ }.Rotate(q1);
		camera->ahead = aheadV;

		const float pitch = -asinf(std::clamp(camera->ahead.y, -0.995f, 0.995f));
		rifleOriginTransform->rotation = Netcode::Quaternion{ pitch, 0.0f, 0.0f };

		controller->setFootPosition(ToPxExtVec3(transform->position));
	}
}
