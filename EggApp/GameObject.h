#pragma once

#include <Egg/HandleTypes.h>
#include "ConstantBufferTypes.h"
#include <vector>
#include <Egg/EggMpl.hpp>
#include <tuple>
#include <memory>
#include <Egg/Logger.h>
#include <Egg/Input.h>
#include <Egg/Blackboard.h>
#include "Material.h"
#include "Mesh.h"
#include <Egg/PhysXWrapper.h>
#include "ComponentStorage.hpp"
#include "Components.h"

using Components_T = std::tuple<Transform, Model, Script, Collider>;
using ExtensionComponents_T = std::tuple<Camera, Animation>;
using AllComponents_T = TupleMerge<Components_T, ExtensionComponents_T>::type;

enum class GameObjectFlags : uint32_t {
	ENABLED = 1,
	MARKED_FOR_DELETION = 2,
	MARKED_FOR_SPAWN = 4
};

class GameObject {
	ComponentStorage<Components_T, ExtensionComponents_T> components;
	GameObject * parent;
	uint32_t flags;
public:
	inline SignatureType GetSignature() const {
		return components.signature;
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

	GameObject * Parent() const {
		return parent;
	}

	void Parent(GameObject * prt) {
		parent = prt;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
	}

	bool IsDeletable() const {
		return (flags & (static_cast<uint32_t>(GameObjectFlags::MARKED_FOR_DELETION))) != 0;
	}

	bool SetDeletable(bool value) {
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
