#pragma once

#include <functional>
#include <Egg/EggMpl.hpp>
#include "ComponentStorage.hpp"
#include "Components.h"

using UIComponents_T = std::tuple<Transform, UIElement, Sprite, Button, Text>;
using UIExtensionComponents_T = std::tuple<Camera>;

class UIObject {
public:
	using StorageType = ComponentStorage<UIComponents_T, UIExtensionComponents_T>;
	using ComponentTypes = typename StorageType::ALL_COMPONENTS_T;

protected:
	StorageType components;
	UIObject * parent;
	std::vector<UIObject *> children;
	uint32_t flags;
public:
	inline SignatureType GetSignature() const {
		return components.signature;
	}

	UIObject() = default;
	UIObject(const UIObject &) = delete;
	UIObject(UIObject && rhs) noexcept = default;
	UIObject & operator=(UIObject && rhs) noexcept = default;

	UIObject * Parent() const {
		return parent;
	}

	bool IsSpawnable() const {
		return (flags & (0x1)) > 0;
	}

	void SetSpawnableFlag(bool tf) {
		if(tf) {
			flags |= 0x1;
		} else {
			flags &= (~0x1);
		}
	}

	void Spawn() {
		SetSpawnableFlag(true);
	}

	void Parent(UIObject * obj) {
		// @TODO
	}

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

};
