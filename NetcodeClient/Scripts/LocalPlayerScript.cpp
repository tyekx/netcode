#include "LocalPlayerScript.h"
#include "../Services.h"
#include "../Network/GameClient.h"


struct TrailScript : public ScriptBase {
	GameObject * owner;
	Netcode::Module::IGraphicsModule * graphics;
	Netcode::Duration lifetime;
	Netcode::Float3 start;
	Netcode::Float3 end;

	constexpr static Netcode::Duration DURATION = std::chrono::milliseconds(750);

	TrailScript(const Netcode::Float3& start, const Netcode::Float3& end) :
		owner{ nullptr }, graphics{ nullptr }, lifetime{ DURATION },
		start { start }, end{ end } {
		graphics = Service::Get<Netcode::Module::IGraphicsModule *>();
	}

	void BeginPlay(GameObject* gameObject) override {
		owner = gameObject;
	}
	
	void Update(Netcode::GameClock* gameClock) override {
		if(graphics == nullptr || graphics->debug == nullptr) {
			owner->SetDeletable(true);
			return;
		}
		
		lifetime -= gameClock->GetDeltaTime();

		if(lifetime < Netcode::Duration{}) {
			owner->SetDeletable(true);
		} else {
			const Netcode::Vector3 maxColor = Netcode::Float3{ 0.8f, 0.8f, 0.8f };
			const Netcode::Vector3 minColor = Netcode::Float3{ 0.4f, 0.4f, 0.4f };
			const float alpha =
				std::chrono::duration<float>(lifetime).count() / 
				std::chrono::duration<float>(DURATION).count();

			graphics->debug->DrawLine(start, end, Netcode::Vector3::Lerp(minColor, maxColor, alpha));
		}
	}
};

static GameObject* SpawnTrail(GameScene* scene, const Netcode::Float3 & start, const Netcode::Float3 & end) {
	GameObject* obj = scene->Create();

	Script * scr = obj->AddComponent<Script>();
	scr->AddScript(std::make_unique<TrailScript>(start, end));

	scene->Spawn(obj);
	
	return obj;
}

void LocalPlayerScript::UpdateLookDirection(float dt) {
	const Netcode::Int2 mouseDelta = Netcode::Input::GetMouseDelta();

	const Netcode::Float2 normalizedMouseDelta{ -(float)(mouseDelta.x), -(float)(mouseDelta.y) };
	cameraPitch -= mouseSpeed * normalizedMouseDelta.y * dt;
	cameraPitch = std::clamp(cameraPitch, -(DirectX::XM_PIDIV2 - 0.0001f), (DirectX::XM_PIDIV2 - 0.0001f));

	cameraYaw += mouseSpeed * normalizedMouseDelta.x * dt;

	if(cameraYaw < (-DirectX::XM_PI)) {
		cameraYaw += DirectX::XM_2PI;
	}

	if(cameraYaw >(DirectX::XM_PI)) {
		cameraYaw -= DirectX::XM_2PI;
	}

	const Netcode::Quaternion cameraQuat{ cameraPitch, cameraYaw, 0.0f };
	const Netcode::Vector3 aheadStart = Netcode::Float3{ 0.0f, 0.0f, 1.0f };
	camera->ahead = aheadStart.Rotate(cameraQuat).Normalize();
	
	transform->rotation = Netcode::Quaternion{ 0.0f, cameraYaw, 0.0f };
	attachmentTransform->rotation = Netcode::Quaternion{ cameraPitch, 0.0f, 0.0f };
}

void LocalPlayerScript::BeginPlay(GameObject * owner) {
	transform = owner->GetComponent<Transform>();
	controller->setPosition(physx::PxExtendedVec3{ transform->position.x, transform->position.y, transform->position.z });
	collider = owner->GetComponent<Collider>();
	leftClickHeld = false;
	reloadTimer = Netcode::Duration{ };
	testAutoFireTimer = std::chrono::seconds(3);
	scene = Service::Get<GameSceneManager>()->GetScene();
}

void LocalPlayerScript::HandleFireInput(Netcode::GameClock * gameClock) {
	float fireAxis = Netcode::Input::GetAxis(FIRE1);

	if(reloadTimer > Netcode::Duration{}) {
		reloadTimer -= gameClock->GetDeltaTime();
		return;
	}

	if(testAutoFireTimer < Netcode::Duration{}) {
		testAutoFireTimer = std::chrono::seconds(3);
		fireAxis = 1.0f; leftClickHeld = false;
	} else {
		testAutoFireTimer -= gameClock->GetDeltaTime();
	}
	
	if(!leftClickHeld && fireAxis == 1.0f) {
		leftClickHeld = true;

		const Netcode::Vector3 worldPos = transform->worldPosition;
		const Netcode::Vector3 raySource = worldPos + Netcode::Float3{ 0.0f, 160.0f, 0.0f };
		const Netcode::Vector3 rayDir = camera->ahead;

		constexpr float maxDistance = 10000.0f;
		
		reloadTimer = std::chrono::seconds(1);

		physx::PxScene * pxScene = scene->GetPhysXScene();
		physx::PxRaycastBuffer hit;
		physx::PxQueryFilterData fd;
		fd.flags |= physx::PxQueryFlag::eANY_HIT;
		fd.data.word0 = PHYSX_COLLIDER_TYPE_LOCAL_HITBOX;
		fd.data.word1 = PHYSX_COLLIDER_TYPE_WORLD | PHYSX_COLLIDER_TYPE_CLIENT_HITBOX;
		fd.data.word2 = 0;
		fd.data.word3 = 0;
		
		bool status = pxScene->raycast(ToPxVec3(raySource), ToPxVec3(rayDir), maxDistance, hit, physx::PxHitFlag::eDEFAULT, fd);

		Netcode::Float3 pos;

		if(!status) {
			pos = raySource + rayDir * maxDistance;
		} else if(!hit.hasBlock) {
			Log::Debug("Unexpected");
			return;
		} else {
			pos = Netcode::ToFloat3(hit.block.position);
			Netcode::Float3 normal = Netcode::ToFloat3(hit.block.normal);
		}

		SpawnTrail(scene, raySource, pos);

		client->SendAction(ClientAction::Fire(ClientFireActionData{ raySource, rayDir }));
	}

	if(fireAxis == 0.0f) {
		leftClickHeld = false;
	}
}

void LocalPlayerScript::Update(Netcode::GameClock * gameClock) {
	const float dt = gameClock->FGetDeltaTime();
	UpdateLookDirection(dt);

	HandleFireInput(gameClock);

	Netcode::Vector3 lDirectionalMovement = GetDirectionalMovement();
	
	// debug movement for the connected clients
	if(!IsDebuggerPresent()) {
		if(transform->position.z > 750.0f) {
			debugMoveLeft = false;
		}

		if(transform->position.z < -750.0f) {
			debugMoveLeft = true;
		}

		lDirectionalMovement = (debugMoveLeft) ? Netcode::Float3{ 0.0f, 0.0f, 1.0f } : Netcode::Float3{ 0.0f, 0.0f, -1.0f };
		cameraYaw = 0.0f;
	}

	const Netcode::Quaternion cameraYawQuat{ 0.0f, cameraYaw, 0.0f };
	const Netcode::Vector3 movementDeltaWorldSpace = lDirectionalMovement.Rotate(cameraYawQuat);
	const Netcode::Vector3 movementDeltaSpeedScaled = movementDeltaWorldSpace * avatarSpeed * dt;
	const Netcode::Vector3 velocityVector = velocity;
	
	const Netcode::Vector3 movementDelta = movementDeltaSpeedScaled + velocityVector * dt;

	if(!movementDelta.AllZero()) {
		physx::PxFilterData fd;
		fd.word1 = PHYSX_COLLIDER_TYPE_WORLD | PHYSX_COLLIDER_TYPE_KILLZONE | PHYSX_COLLIDER_TYPE_CLIENT_HITBOX | PHYSX_COLLIDER_TYPE_LOCAL_HITBOX;
		fd.word0 = PHYSX_COLLIDER_TYPE_LOCAL_HITBOX;
		fd.word2 = 0;
		fd.word3 = 0;
		
		physx::PxControllerFilters filters;
		filters.mFilterData = &fd;
		
		const physx::PxControllerCollisionFlags moveResult = controller->move(ToPxVec3(movementDelta), 0.0f, dt, filters);

		if(moveResult == physx::PxControllerCollisionFlag::eCOLLISION_DOWN) {
			velocity.y = 0.0f;
		}

		const auto footPos = controller->getFootPosition();

		transform->position = Netcode::Float3{
			static_cast<float>(footPos.x),
			static_cast<float>(footPos.y),
			static_cast<float>(footPos.z)
		};
	}
}

void LocalPlayerScript::FixedUpdate(Netcode::GameClock * gameClock) {
	const float dt = gameClock->FGetFixedDeltaTime();
	const Netcode::Vector3 gravityVector = gravity;
	const Netcode::Vector3 velocityVector = velocity;
	const Netcode::Vector3 gravityDeltaVelocity = gravityVector * dt;
	velocity = velocityVector + gravityDeltaVelocity;
}
