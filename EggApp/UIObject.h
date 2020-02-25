#pragma once

#include <functional>
#include <Egg/EggMpl.hpp>
#include "ComponentStorage.hpp"
#include "Components.h"

using UIComponents_T = std::tuple<Transform, Sprite, Button, Text>;
using UIExtensionComponents_T = std::tuple<Camera, LongText>;

class UIObject {
protected:
	ComponentStorage<UIComponents_T, UIExtensionComponents_T> components;
	UIObject * parent;
	std::vector<UIObject *> children;
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
