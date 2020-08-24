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

using Components_T = std::tuple<Transform, Model, Script, Collider>;
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
	uint32_t flags;

public:
	const char * debugName;

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
