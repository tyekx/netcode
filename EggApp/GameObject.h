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
#include <Egg/PhysXScene.h>
#include <physx/characterkinematic/PxCapsuleController.h>

class GameObject;
using ColliderShape = Egg::Asset::Collider;

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

	Script() = default;

	Script(Script &&) noexcept = default;
	Script & operator=(Script &&) noexcept = default;
};

COMPONENT_ALIGN class Transform {
public:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 rotation;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 worldPosition;
	DirectX::XMFLOAT4 worldRotation;

	Transform() : position{ 0.0f, 0.0f, 0.0f }, rotation{ 0.0f, 0.0f, 0.0f, 1.0f }, scale{ 1.0f, 1.0f, 1.0f }, worldPosition{ position }, worldRotation{ rotation } {

	}

	Transform(Transform &&) noexcept = default;
	Transform & operator=(const Transform &) = default;
	Transform & operator=(Transform &&) noexcept = default;
	~Transform() noexcept = default;
};

struct ShadedMesh {
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;

	ShadedMesh(std::shared_ptr<Mesh> m, std::shared_ptr<Material> mat) : mesh{ std::move(m) }, material{ std::move(mat) } {

	}

	ShadedMesh(ShadedMesh &&) noexcept = default;
	~ShadedMesh() noexcept = default;
	ShadedMesh & operator=(ShadedMesh&& sm) noexcept = default;
};

COMPONENT_ALIGN class Model {
public:

	PerObjectData perObjectData;
	std::unique_ptr<BoneData> boneData;
	std::vector<ShadedMesh> meshes;

	Model() : perObjectData{ }, boneData{ nullptr }, meshes{ } { }
	~Model() = default;

	Model(Model &&) noexcept = default;
	Model & operator=(Model&& m) noexcept = default;

	ShadedMesh & AddShadedMesh(std::shared_ptr<Mesh> m, std::shared_ptr<Material> mat) {
		return meshes.emplace_back(std::move(m), std::move(mat));
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

	Camera & operator=(const Camera &) = default;
};

COMPONENT_ALIGN class Animation {
public:
	Egg::Animation::Blackboard blackboard;

	Animation() = default;
	Animation(Animation &&) noexcept = default;
	Animation & operator=(Animation &&) noexcept = default;
};

COMPONENT_ALIGN class Collider {
public:
	physx::PxActor * actorRef;
	std::vector<ColliderShape> shapes;

	Collider() = default;
	Collider(Collider &&) noexcept = default;

	Collider & operator=(Collider&& c) noexcept = default;
};

template<typename TUPLE_T, typename TUPLE_T2>
struct TupleMoveStorageImpl;

template<typename TUPLE_T>
struct TupleMoveStorageImpl<TUPLE_T, std::tuple<>> {
	static void Invoke(uint8_t * dst, uint8_t * src, SignatureType sig) {

	}
};

template<typename TUPLE_T, typename HEAD, typename ... TAIL>
struct TupleMoveStorageImpl<TUPLE_T, std::tuple<HEAD, TAIL...>> {
	static void Invoke(uint8_t * dst, uint8_t * src, SignatureType sig) {
		using ComponentType = HEAD;
		constexpr static uint32_t offsetOf = TupleOffsetOf<ComponentType, TUPLE_T>::value;
		constexpr static SignatureType mask = TupleCreateMask<std::tuple<ComponentType>, TUPLE_T>::value;

		if((sig & mask) != 0ull) {
			ComponentType * lhs = reinterpret_cast<ComponentType *>(dst + offsetOf);
			ComponentType * rhs = reinterpret_cast<ComponentType *>(src + offsetOf);
			std::swap(*lhs, *rhs);
		}

		TupleMoveStorageImpl<TUPLE_T, std::tuple<TAIL...>>::Invoke(dst, src, sig);
	}
};

template<typename T>
struct TupleMoveStorage {
	static void Invoke(uint8_t * dst, uint8_t* src, SignatureType signature) {
		TupleMoveStorageImpl< T, T >::Invoke(dst, src, signature);
	}
};

using Components_T = std::tuple<Transform, Model, Script, Collider>;
using ExtensionComponents_T = std::tuple<Camera, Animation>;
using AllComponents_T = TupleMerge<Components_T, ExtensionComponents_T>::type;

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

	ComponentStorage() = default;
	~ComponentStorage() noexcept {
		CompositeObjectDestructor<Components_T>::Invoke(signature, storage);
		if(extendedStorage != nullptr) {
			signature >>= TupleCountOf<Components_T>::value;
			CompositeObjectDestructor<ExtensionComponents_T>::Invoke(signature, extendedStorage);
			std::free(extendedStorage);
			extendedStorage = nullptr;
		}
	}

	ComponentStorage & operator=(ComponentStorage rhs) noexcept {
		constexpr static uint32_t StorageSize = TupleSizeofSum<Components_T>::value;
		uint8_t tempStorage[StorageSize];

		SignatureType signatureMask = TupleMaxMaskValue<Components_T>::value;
		SignatureType mainStorageSignature = signature & signatureMask;
		SignatureType rhsMainStorageSignature = rhs.signature & signatureMask;

		TupleMoveStorage<Components_T>::Invoke(tempStorage, storage, mainStorageSignature);
		TupleMoveStorage<Components_T>::Invoke(storage, rhs.storage, rhsMainStorageSignature);
		TupleMoveStorage<Components_T>::Invoke(rhs.storage, tempStorage, mainStorageSignature);

		std::swap(signature, rhs.signature);
		std::swap(extendedStorage, rhs.extendedStorage);
		return *this;
	}

	template<typename T>
	bool HasComponent() {
		static_assert(TupleContainsType<T, AllComponents_T>::value, "Component type was not found");

		if constexpr(TupleContainsType<T, Components_T>::value) {
			return (signature & (1ULL << TupleIndexOf<T, Components_T>::value)) > 0;
		} else {
			return HasExtensionComponent<T>();
		}
	}

	template<typename T>
	T * GetComponent() {
		static_assert(TupleContainsType<T, AllComponents_T>::value, "Component type was not found");

#if _DEBUG
		SignatureType sigMask = 1ull << TupleIndexOf<T, AllComponents_T>::value;
		// force a crash in debug mode
		if((signature & sigMask) == 0) {
			return nullptr;
		}
#endif

		if constexpr(TupleContainsType<T, Components_T>::value) {
			return reinterpret_cast<T *>(storage + TupleOffsetOf<T, Components_T>::value);
		} else {
			return GetExtensionComponent<T>();
		}
	}

	template<typename T>
	void RemoveComponent() {
		static_assert(TupleContainsType<T, AllComponents_T>::value, "Component type was not found");

		T * component = GetComponent<T>();
		
		component->~T();
	}

	template<typename T>
	T * AddComponent() {
		static_assert(TupleContainsType<T, AllComponents_T>::value, "Component type was not found");

		if constexpr(TupleContainsType<T, Components_T>::value) {
			if(!HasComponent<T>()) {
				signature |= (1ULL << TupleIndexOf<T, Components_T>::value);
				T * ptr = GetComponent<T>();
				new (ptr) T();
				return ptr;
			}
			return GetComponent<T>();
		} else {
			return AddExtensionComponent<T>();
		}
	}
};

enum class GameObjectFlags : uint32_t {
	ENABLED = 1,
	MARKED_FOR_DELETION = 2,
	MARKED_FOR_SPAWN = 4
};

class GameObject {
	ComponentStorage components;
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
