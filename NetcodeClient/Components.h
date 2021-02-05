#pragma once

#include <NetcodeFoundation/Math.h>

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
#include "Network/ReplDesc.h"
#include <variant>

namespace nn = Netcode::Network;
namespace np = Netcode::Protocol;

namespace Netcode {
	class GameClock;
}

namespace Netcode::Network {
	class ReplicationContext;
}

enum class ActionType {
	NOOP = 0, MOVEMENT = 1, FIRE = 2, JUMP = 3, SPAWN = 4
};

enum class PlayerState : uint32_t {
	SPECTATOR,
	ALIVE,
	DEAD,
	DISABLED /* server value */
};

struct ClientLookActionData {
	float pitch;
	float yaw;
};

struct ClientFireActionData {
	Netcode::Float3 position;
	Netcode::Float3 direction;
};

struct ClientMovementActionData {
	Netcode::Float3 position;
};

struct ClientAction {
	uint32_t id;
	ActionType type;
	Netcode::Timestamp timestamp;

	union {
		ClientLookActionData lookActionData;
		ClientMovementActionData movementActionData;
		ClientFireActionData fireActionData;
	};

	NETCODE_CONSTRUCTORS_ALL(ClientAction);

	static ClientAction Fire(const ClientFireActionData & fireAction) {
		ClientAction ca;
		ca.id = 0;
		ca.timestamp = Netcode::SystemClock::GlobalNow();
		ca.type = ActionType::FIRE;
		ca.fireActionData = fireAction;
		return ca;
	}

	static ClientAction Move(const ClientMovementActionData & actionData) {
		ClientAction ca;
		ca.id = 0;
		ca.timestamp = Netcode::SystemClock::GlobalNow();
		ca.type = ActionType::MOVEMENT;
		ca.movementActionData = actionData;
		return ca;
	}

	static ClientAction Jump() {
		ClientAction ca;
		ca.id = 0;
		ca.timestamp = Netcode::SystemClock::GlobalNow();
		ca.type = ActionType::JUMP;
		return ca;
	}

	static ClientAction Spawn() {
		ClientAction ca;
		ca.id = 0;
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
	ACCEPTED, REJECTED, COMMAND
};

enum class CommandType {
	NOOP = 0,
	PLAYER_KICKED = 1,
	PLAYER_BANNED = 2,
	PLAYER_CONNECTED = 3,
	PLAYER_DISCONNECTED = 4,
	PLAYER_TIMEDOUT = 5,
	SERVER_CLOSED = 6,
	CREATE_OBJECT = 7,
	REMOVE_OBJECT = 8
};

struct ServerCommand {
	CommandType type;
	int32_t subject;
	int32_t objectType;
	uint32_t objectId;

	static ServerCommand CreateObject(int32_t ownerId, int32_t type, uint32_t objId) {
		ServerCommand scmd;
		scmd.objectType = type;
		scmd.subject = ownerId;
		scmd.objectId = objId;
		scmd.type = CommandType::CREATE_OBJECT;
		return scmd;
	}
};

struct ServerReconciliation {
	uint32_t id;
	ReconciliationType type;
	ActionType actionType;
	std::string replData;

	union {
		ClientMovementActionData movementCorrection;
		ServerCommand command;
	};

	static ServerReconciliation CreateCommand(uint32_t cmdId, ServerCommand cmd) {
		ServerReconciliation sr;
		sr.id = cmdId;
		sr.type = ReconciliationType::COMMAND;
		sr.actionType = ActionType::NOOP;
		sr.command = cmd;
		return sr;
	}
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

struct RedundancyItem {
	uint32_t sequence;
	std::variant<ClientAction, ServerReconciliation> storage;
};

/*
 * Mitigation for packetloss. This includes every action or result that is not handled yet
 */
class RedundancyBuffer {
	std::vector<RedundancyItem> items;
	
public:
	void Add(uint32_t localSequence, ClientAction action) {
		RedundancyItem item = {};
		item.sequence = localSequence;
		item.storage.emplace<ClientAction>(action);
		items.push_back(item);
	}

	void Add(uint32_t localSequence, ServerReconciliation reconciliation) {
		RedundancyItem item = {};
		item.sequence = localSequence;
		item.storage.emplace<ServerReconciliation>(std::move(reconciliation));
		items.push_back(item);
	}

	const std::vector<RedundancyItem> & GetBuffer() const {
		return items;
	}

	// remove objects that are implicitly confirmed by the remoteSequence.
	void Confirm(uint32_t confirmedSequence) {
		if(items.empty())
			return;
		
		auto it = std::remove_if(std::begin(items), std::end(items), [confirmedSequence](const RedundancyItem & i) -> bool {
			return i.sequence <= confirmedSequence;
		});
		
		items.erase(it, std::end(items));
	}
};

template<typename T>
struct HistoryItem {
	Netcode::Timestamp timestamp;
	T value;
};

template<typename T>
class HistoryBuffer {

	std::unique_ptr<HistoryItem<T>[]> data;
	int32_t index;
	int32_t size;
	int32_t capacity;

	int32_t Increment() {
		const int32_t idx = index;
		index++;
		if(index >= capacity) {
			index = 0;
		}

		if(size < capacity) {
			size += 1;
		}
		return idx;
	}
	
public:
	HistoryBuffer(int32_t numElements) : data{}, index { 0 }, capacity{ numElements }, size{ 0 } {
		data = std::make_unique<HistoryItem<T>[]>(numElements);
	}
	
	void Insert(Netcode::Timestamp timestamp, const T& value) {
		const int32_t idx = Increment();
		
		data[idx] = HistoryItem<T>{
			timestamp,
			value
		};
	}

	/*
	 * Possible return values:
	 * - no intersection: { nullptr, nullptr }
	 * - only newer items: { ptr, nullptr }
	 * - has expected intersection: { ptr, ptr }
	 *
	 * should never return: { nullptr, ptr }
	 */
	std::pair<const HistoryItem<T>*, const HistoryItem<T> *> FindAt(const Netcode::Timestamp& timestamp) {
		const HistoryItem<T> * first = nullptr;
		const HistoryItem<T> * last = nullptr;

		if(size > 0) {

			int32_t idx = index - 1;
			for(int32_t i = 0; i < size; i++, idx--) {
				// underflow case
				if(idx < 0) {
					idx += capacity;
				}

				const HistoryItem<T> * bufferItem = data.get() + idx;

				if(bufferItem->timestamp > timestamp) {
					first = bufferItem;
				}
				
				if(bufferItem->timestamp <= timestamp) {
					last = bufferItem;
					// even the first item is older, therefore nothing was found
					if(first == nullptr) {
						last = nullptr;
					}
					// older ones down the road, no reason to keep searching
					break;
				}
			}
			
		}

		return std::pair<const HistoryItem<T> *, const HistoryItem<T>*>{ first, last };
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
	virtual void Construct(GameObject * gameObject) { }
	virtual void BeginPlay(GameObject * gameObject) { }
	virtual void EndPlay() { }
	virtual void Update(Netcode::GameClock * gameClock) { }
	virtual void FixedUpdate(Netcode::GameClock * gameClock) { }
};

COMPONENT_ALIGN class Script {
public:
	std::vector<std::unique_ptr<ScriptBase>> scripts;

	void AddScript(std::unique_ptr<ScriptBase> script) {
		scripts.emplace_back(std::move(script));
	}

	template<typename T = ScriptBase>
	T* GetScript(uint32_t idx) {
		return dynamic_cast<T *>(scripts[idx].get());
	}

	void BeginPlay(GameObject * owner) {
		for(auto it = std::begin(scripts); it != std::end(scripts); it++) {
			(*it)->BeginPlay(owner);
		}
	}
	
	void Update(Netcode::GameClock * clock) {
		for(auto it = std::begin(scripts); it != std::end(scripts); it++) {
			(*it)->Update(clock);
		}
	}

	void FixedUpdate(Netcode::GameClock * clock) {
		for(auto it = std::begin(scripts); it != std::end(scripts); it++) {
			(*it)->FixedUpdate(clock);
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

COMPONENT_ALIGN struct Network {
	uint32_t id;
	int32_t owner;
	Netcode::Timestamp updatedAt;
	PlayerState state;
	ReplDesc replDesc;

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
