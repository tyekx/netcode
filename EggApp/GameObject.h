#pragma once

#include <Egg/HandleTypes.h>
#include "ConstantBufferTypes.h"
#include <vector>
#include <Egg/EggMpl.hpp>
#include <tuple>
#include <Egg/Logger.h>
#include <Egg/Input.h>
#include <Egg/Blackboard.h>

class GameObject;

#define COMPONENT_ALIGN __declspec(align(16))

class IBehavior {
public:
	virtual ~IBehavior() = default;
	virtual void Setup(GameObject* gameObject) = 0;
	virtual void Update(float dt) = 0;
};

COMPONENT_ALIGN class Script {
protected:
	std::unique_ptr<IBehavior> behavior;
public:
	void SetBehavior(std::unique_ptr<IBehavior> behavior);
	void Setup(GameObject * owner);
	void Update(float dt);
};

COMPONENT_ALIGN class Transform {
public:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 rotation;
	DirectX::XMFLOAT3 scale;

	Transform() : position{ 0.0f, 0.0f, 0.0f }, rotation{ 0.0f, 0.0f, 0.0f, 1.0f }, scale{ 1.0f, 1.0f, 1.0f } {

	}
};

COMPONENT_ALIGN class Model {
public:
	struct ShadedMesh {
		int vertexBuffer;
		int indexBuffer;
		MaterialData materialData;

		ShadedMesh(int vbuffer, int ibuffer) : vertexBuffer{ vbuffer }, indexBuffer{ ibuffer }, materialData{ } { }
	};

	PerObjectData perObjectData;
	std::unique_ptr<BoneData> boneData;
	std::vector<ShadedMesh> meshes;

	Model() : perObjectData{ }, boneData{ nullptr }, meshes{ } { }
	~Model() = default;

	ShadedMesh & AddShadedMesh(int vertexBuffer, int indexBuffer = -1) {
		return meshes.emplace_back(vertexBuffer, indexBuffer);
	}

};


COMPONENT_ALIGN class Camera {
public:
	DirectX::XMFLOAT3 ahead;
	DirectX::XMFLOAT3 up;

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
};

COMPONENT_ALIGN class Animation {
public:
	Egg::Animation::Blackboard blackboard;
};

using Components_T = std::tuple<Transform, Model, Script>;
using ExtensionComponents_T = std::tuple<Camera, Animation>;

class ComponentStorage {
	uint8_t storage[TupleSizeofSum<Components_T>::value];
	uint8_t * extendedStorage;

	void InitExtendedStorage() {
		if(extendedStorage == nullptr) {
			extendedStorage = reinterpret_cast<uint8_t *>(std::malloc(TupleSizeofSum<ExtensionComponents_T>::value));
		}
	}

	template<typename T>
	bool HasExtensionComponent() {
		constexpr SignatureType componentIdx = TupleCountOf<Components_T>::value + TupleIndexOf<T, ExtensionComponents_T>::value;
		return (signature & (1ULL << componentIdx)) > 0;
	}

	template<typename T>
	T * GetExtensionComponent() {
		return reinterpret_cast<T *>(extendedStorage + TupleOffsetOf<T, ExtensionComponents_T>::value);
	}

	template<typename T>
	T * AddExtensionComponent() {
		if(!HasExtensionComponent<T>()) {
			InitExtendedStorage();
			constexpr SignatureType componentIdx = TupleCountOf<Components_T>::value + TupleIndexOf<T, ExtensionComponents_T>::value;
			T * ptr = GetExtensionComponent<T>();
			signature |= (1ULL << componentIdx);
			new (ptr) T();
			return ptr;
		}
		return GetExtensionComponent<T>();
	}

public:
	SignatureType signature;


	ComponentStorage() : signature{}, storage{}, extendedStorage{ nullptr } { }
	~ComponentStorage() {
		CompositeObjectDestructor<Components_T>::Invoke(signature, storage);
		if(extendedStorage != nullptr) {
			signature >>= TupleCountOf<Components_T>::value;
			CompositeObjectDestructor<ExtensionComponents_T>::Invoke(signature, extendedStorage);
			std::free(extendedStorage);
			extendedStorage = nullptr;
		}
	}

	template<typename T>
	bool HasComponent() {
		if constexpr(TupleContainsType<T, Components_T>::value) {
			return (signature & (1ULL << TupleIndexOf<T, Components_T>::value)) > 0;
		} else {
			return HasExtensionComponent<T>();
		}
	}

	template<typename T>
	T * GetComponent() {
		if constexpr(TupleContainsType<T, Components_T>::value) {
			return reinterpret_cast<T *>(storage + TupleOffsetOf<T, Components_T>::value);
		} else {
			return GetExtensionComponent<T>();
		}
	}

	template<typename T>
	T * AddComponent() {
		if constexpr(TupleContainsType<T, Components_T>::value) {
			if(!HasComponent<T>()) {
				T * ptr = GetComponent<T>();
				signature |= (1ULL << TupleIndexOf<T, Components_T>::value);
				new (ptr) T();
				return ptr;
			}
			return GetComponent<T>();
		} else {
			return AddExtensionComponent<T>();
		}
	}
};

class GameObject {
	ComponentStorage components;
	GameObject * parent;
	bool disabled;
public:
	inline SignatureType GetSignature() const {
		return components.signature;
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

	GameObject * Parent() const {
		return parent;
	}

	void Parent(GameObject * prt) {
		parent = prt;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
	}

	bool IsActive() const {
		return !disabled;
	}

	void SetActive(bool value) {
		disabled = !value;
	}
};


class PlayerBehavior : public IBehavior {
	Transform * transform;
public:
	virtual void Setup(GameObject * owner) override {
		transform = owner->GetComponent<Transform>();
		

	}

	virtual void Update(float dt) override {
		//float vertical = Egg::Input::GetAxis("Vertical");

		//transform->position.z += vertical * 250.0f * dt;
	}

};
