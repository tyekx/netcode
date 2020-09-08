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

namespace Netcode::Network {
	class ReplicationContext;
}

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
	virtual void ReplicateSend(Netcode::Network::ReplicationContext * ctx) { }
	virtual void ReplicateReceive(Netcode::Network::ReplicationContext * ctx) { }
};

COMPONENT_ALIGN class Script {
public:
	std::vector<std::unique_ptr<ScriptBase>> scripts;

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
	std::vector<ShadedMesh> meshes;
	std::vector<Ref<Netcode::Material>> materials;

	ShadedMesh & AddShadedMesh(Ref<Mesh> m, Ref<Netcode::Material> mat) {
		return meshes.emplace_back(std::move(m), std::move(mat));
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
