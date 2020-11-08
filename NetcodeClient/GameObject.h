#pragma once

#include <Netcode/HandleTypes.h>
#include "ConstantBufferTypes.h"
#include <vector>
#include <Netcode/NetcodeMpl.hpp>
#include <tuple>
#include <memory>
#include <Netcode/Logger.h>
#include <Netcode/Input.h>
#include "Mesh.h"
#include <Netcode/PhysXWrapper.h>
#include "ComponentStorage.hpp"
#include "Components.h"

using Components_T = std::tuple<Transform, Model, Netcode::Light, Script, Collider, Netw>;
using ExtensionComponents_T = std::tuple<Camera, Animation>;

enum class GameObjectFlags : uint32_t {
	ENABLED = 1,
	MARKED_FOR_DELETION = 2,
	MARKED_FOR_SPAWN = 4
};

class GameObject {
public:
	using StorageType = ComponentStorage<Components_T, ExtensionComponents_T>;
	using ComponentTypes = typename StorageType::ALL_COMPONENTS_T;

protected:
	StorageType components;
	GameObject * parent;
	std::vector<GameObject *> children;
	uint32_t flags;

	inline void AddChildDetail(GameObject * child) {
		children.push_back(child);
	}

	inline void RemoveChildDetail(GameObject * child) {
		auto it = std::find(std::begin(children), std::end(children), child);
		if(it != std::end(children)) {
			children.erase(it);
			child->parent = nullptr;
		}
	}
public:
	std::string name;

	inline SignatureType GetSignature() const {
		return components.signature;
	}

	const std::vector<GameObject *> & Children() const {
		return children;
	}

	GameObject() = default;
	GameObject(const GameObject &) = delete;
	GameObject(GameObject && rhs) noexcept = default;
	GameObject & operator=(GameObject && rhs) noexcept = default;

	template<typename T>
	bool HasComponent() {
		return components.HasComponent<T>();
	}

	template<typename T>
	T * AddComponent() {
		return components.AddComponent<T>();
	}

	template<typename T>
	T * GetComponent() {
		return components.GetComponent<T>();
	}

	template<typename ... T>
	std::tuple<T*...> AddComponents() {
		return std::tuple<T*...>(AddComponent<T>()...);
	}

	GameObject * Parent() const {
		return parent;
	}

	void AddChild(GameObject * obj) {
		if(obj != nullptr) {
			if(obj->parent != nullptr) {
				obj->parent->RemoveChildDetail(obj);
			}
			obj->parent = this;
			AddChildDetail(obj);
		}
	}

	void Parent(GameObject * prnt) {
		if(parent != nullptr) {
			parent->RemoveChildDetail(this);
		}
		parent = prnt;
		if(parent != nullptr) {
			parent->AddChildDetail(this);
		}
	}

	bool IsDeletable() const {
		return (flags & (static_cast<uint32_t>(GameObjectFlags::MARKED_FOR_DELETION))) != 0;
	}

	void SetDeletable(bool value) {
		if(value) {
			flags |= static_cast<uint32_t>(GameObjectFlags::MARKED_FOR_DELETION);
		} else {
			flags &= ~(static_cast<uint32_t>(GameObjectFlags::MARKED_FOR_DELETION));
		}
	}

	bool IsSpawnable() const {
		return (flags & (static_cast<uint32_t>(GameObjectFlags::MARKED_FOR_SPAWN))) != 0;
	}

	void Spawn() {
		SetActive(true);
		flags &= ~(static_cast<uint32_t>(GameObjectFlags::MARKED_FOR_SPAWN));
	}

	bool IsActive() const {
		return (flags & (static_cast<uint32_t>(GameObjectFlags::ENABLED))) != 0;
	}

	void SetActive(bool value) {
		if(value) {
			flags |= static_cast<uint32_t>(GameObjectFlags::ENABLED);
		} else {
			flags &= ~(static_cast<uint32_t>(GameObjectFlags::ENABLED));
		}
	}
};

class NatvisComponentObject {
	Transform transform;
	Model model;
	Netcode::Light light;
	Script script;
	Collider collider;
	void * rawExtended;
	SignatureType signature;
};

class NatvisExtComponentObject {
	Camera camera;
	Animation animation;
};
