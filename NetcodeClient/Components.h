#pragma once

#include <NetcodeFoundation/Math.h>

#include <functional>
#include <string>
#include "ConstantBufferTypes.h"
#include <NetcodeAssetLib/Collider.h>
#include <NetcodeAssetLib/Bone.h>
#include <NetcodeAssetLib/Animation.h>
#include <Netcode/Input.h>
#include <Netcode/Event.hpp>
#include <Netcode/HandleTypes.h>
#include <Netcode/PhysXWrapper.h>
#include "Mesh.h"
#include "AnimationController.h"

#include <Netcode/Animation/IK.h>
#include <Netcode/Animation/Blackboard.h>
#include <Netcode/Animation/Blender.h>
#include <Netcode/Graphics/Material.h>
#include <Netcode/System/TimeTypes.h>
#include <Netcode/Network/Connection.h>

namespace nn = Netcode::Network;
namespace np = Netcode::Protocol;

namespace Netcode::Network {
	class ReplicationContext;
}

enum class ActionType {
	MOVEMENT, FIRE, JUMP, SPAWN
};

struct ClientPredictedFire {
	Netcode::Float3 position;
	Netcode::Float3 direction;
};

struct ClientPredictedMovement {
	Netcode::Float3 position;
	Netcode::Float3 delta;
};

struct ClientAction {
	static uint32_t idGen;
	uint32_t id;
	ActionType type;
	Netcode::Timestamp timestamp;

	union {
		ClientPredictedMovement movementActionData;
		ClientPredictedFire fireActionData;
	};

	NETCODE_CONSTRUCTORS_ALL(ClientAction);

	static ClientAction Fire(const ClientPredictedFire & fireAction) {
		ClientAction ca;
		ca.id = idGen++;
		ca.timestamp = Netcode::SystemClock::GlobalNow();
		ca.type = ActionType::FIRE;
		ca.fireActionData = fireAction;
		return ca;
	}

	static ClientAction Move(const ClientPredictedMovement & movementAction) {
		ClientAction ca;
		ca.id = idGen++;
		ca.timestamp = Netcode::SystemClock::GlobalNow();
		ca.type = ActionType::MOVEMENT;
		ca.movementActionData = movementAction;
		return ca;
	}

	static ClientAction Jump() {
		ClientAction ca;
		ca.id = idGen++;
		ca.timestamp = Netcode::SystemClock::GlobalNow();
		ca.type = ActionType::JUMP;
		return ca;
	}

	static ClientAction Spawn() {
		ClientAction ca;
		ca.id = idGen++;
		ca.timestamp = Netcode::SystemClock::GlobalNow();
		ca.type = ActionType::SPAWN;
		return ca;
	}
};

class ClientActionBuffer {
	ClientAction movements;
	std::vector<ClientAction> otherTypes;
public:

	ClientActionBuffer() : movements{}, otherTypes{} {
		movements.id = 0;
	}
	// actions that can be merged will be merged here
	void Add(ClientAction action) {
		if(action.type == ActionType::MOVEMENT) {
			movements = action;
		} else {
			otherTypes.push_back(action);
		}
	}

	void Consume(std::vector<ClientAction> & dstBuffer) {
		otherTypes.push_back(movements);
		std::swap(dstBuffer, otherTypes);
	}
};

enum class ReconciliationType {
	ACCEPTED, REJECTED
};

struct ServerReconciliation {
	uint32_t id;
	ReconciliationType type;
	ClientPredictedMovement correctedPosition;
};

class ReconciliationBuffer {
public:
	std::vector<ClientAction> predictions;

	void Reconcile(const np::ActionResult& actionResult) {
		for(const ClientAction & ca : predictions) {
			if(ca.id == actionResult.id()) {
				if(actionResult.result() == np::ActionResultType::ACCEPTED) {
					// ok
				}
			}
		}
	}
};

class InterpolationDelayBuffer {
public:

};


struct ServerActionBuffer {
public:
	const std::vector<ClientAction> & GetActionsFor(int32_t playerId) {

	} 

	void Add(int32_t playerId, ClientAction action) {

	}
	
	void Clear() {
		
	}
};

/*
 * Mitigation for packetloss. This includes every action or result that is not handled yet
 */
class RedundancyBuffer {
public:
	// everything that is sent to the user in frame N
	void Add(uint32_t localSequence, ClientAction action) {

	}

	void Add(uint32_t localSequence, ServerReconciliation reconciliation) {
		
	}

	// apply the data to the replication context
	void Apply(nn::ReplicationContext* ctx) {
		
	}

	// remove objects that are implicitly confirmed by the remoteSequence.
	void Confirm(uint32_t confirmedSequence) {

	}
};

enum AxisEnum : uint32_t {
	VERTICAL,
	HORIZONTAL,
	FIRE1,
	FIRE2,
	JUMP,
	DEV_CAM_X,
	DEV_CAM_Y,
	DEV_CAM_Z
};

using ColliderShape = Netcode::Asset::Collider;

#define COMPONENT_ALIGN __declspec(align(16))

class GameObject;

class ScriptBase {
public:
	virtual ~ScriptBase() = default;
	virtual void Construction(GameObject * gameObject) { }
	virtual void BeginPlay(GameObject * gameObject) { }
	virtual void EndPlay() { }
	virtual void Update(float dt) = 0;
	virtual void ReplicateSend(GameObject * obj, nn::ReplicationContext * ctx) { }
	virtual void ReplicateReceive(GameObject * obj, nn::ReplicationContext * ctx) { }
};

COMPONENT_ALIGN class Script {
public:
	std::vector<std::unique_ptr<ScriptBase>> scripts;

	void ReplicateSend(GameObject * obj, nn::ReplicationContext * ctx) {
		for(auto it = std::begin(scripts); it != std::end(scripts); it++) {
			(*it)->ReplicateSend(obj, ctx);
		}
	}
	
	void ReplicateReceive(GameObject* obj, nn::ReplicationContext * ctx) {
		for(auto it = std::begin(scripts); it != std::end(scripts); it++) {
			(*it)->ReplicateReceive(obj, ctx);
		}
	}

	void AddScript(std::unique_ptr<ScriptBase> script) {
		scripts.emplace_back(std::move(script));
	}

	void BeginPlay(GameObject * owner) {
		for(auto it = std::begin(scripts); it != std::end(scripts); it++) {
			(*it)->BeginPlay(owner);
		}
	}
	
	void Update(float dt) {
		for(auto it = std::begin(scripts); it != std::end(scripts); it++) {
			(*it)->Update(dt);
		}
	}
};

COMPONENT_ALIGN class Transform {
public:
	Netcode::Float3 position;
	Netcode::Float4 rotation;
	Netcode::Float3 scale;
	Netcode::Float3 worldPosition;
	Netcode::Float4 worldRotation;

	Transform();

	physx::PxTransform LocalToPhysX() const;
	physx::PxTransform WorldToPhysX() const;
};

struct ShadedMesh {
	Ref<Mesh> mesh;
	Ref<Netcode::Material> material;

	ShadedMesh() = default;
	ShadedMesh(Ref<Mesh> m, Ref<Netcode::Material> mat) : mesh{ std::move(m) }, material{ std::move(mat) } {

	}
};

COMPONENT_ALIGN class Model {
public:
	PerObjectData perObjectData;
	Ref<Netcode::ResourceViews> boneData;
	int32_t boneDataOffset;
	int32_t renderLayer;
	std::vector<ShadedMesh> meshes;
	std::vector<Ref<Netcode::Material>> materials;

	ShadedMesh & AddShadedMesh(Ref<Mesh> m, Ref<Netcode::Material> mat) {
		return meshes.emplace_back(std::move(m), std::move(mat));
	}
};

COMPONENT_ALIGN struct Netw {
	int32_t owner;
	

	bool HasAuthority(int32_t id) const {
		return owner == id;
	}
};

COMPONENT_ALIGN class Camera {
public:
	Netcode::Float3 ahead;
	Netcode::Float3 up;

	float fov;
	float aspect;
	float nearPlane;
	float farPlane;

	Camera() : ahead{}, up{ 0.0f, 1.0f, 0.0f }, fov{ 1.0f }, aspect{ 1.0f }, nearPlane{ 0.0f }, farPlane{ 1.0f } { }

	void SetProj(float fov, float aspect, float nearPlane, float farPlane) {
		this->fov = fov;
		this->aspect = aspect;
		this->nearPlane = nearPlane;
		this->farPlane = farPlane;
	}

	Camera & operator=(const Camera &) = default;
};

COMPONENT_ALIGN class Animation {
public:
	std::vector<Netcode::Animation::IKEffector> effectors;
	Ref<AnimationController> controller;
	Ref<Netcode::Animation::BlackboardBase> blackboard;
	Ref<Netcode::Animation::Blender> blender;
	std::unique_ptr<BoneData> debugBoneData;
	Netcode::ArrayView<Netcode::Asset::Bone> bones;
	Netcode::ArrayView<Netcode::Asset::Animation> clips;
	Netcode::Quaternion headRotation;

	Animation() = default;
	Animation(Animation &&) noexcept = default;
	Animation & operator=(Animation &&) noexcept = default;
};

COMPONENT_ALIGN class Collider {
public:
	Netcode::PxPtr<physx::PxActor> actor;
	std::vector<ColliderShape> shapes;

	Collider() = default;
};
