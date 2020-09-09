#pragma once

#include "GameObject.h"
#include <list>
#include <memory>
#include <Netcode/BulkVector.hpp>

/*
base class
*/
template<typename T>
class Scene {
protected:
	Netcode::PxPtr<physx::PxScene> pxScene;
	Netcode::PxPtr<physx::PxControllerManager> controllerManager;
	Netcode::BulkVector<T, 512> storage;
	T * activeCamera;
	T * defaultCamera;

public:
	virtual ~Scene() = default;
	Scene() = default;
	Scene(Scene<T> && rhs) noexcept = default;
	Scene & operator=(Scene<T> &&) noexcept = default;
	Scene(const Scene<T> &) = delete;
	Scene & operator=(const Scene<T> &) = delete;

	physx::PxScene * GetPhysXScene() {
		return pxScene.Get();
	}

	void Clear() {
		storage.Clear();
	}

	void SetPhysXScene(Netcode::PxPtr<physx::PxScene> scene) {
		pxScene = std::move(scene);
	}

	T * GetCamera() const {
		return (activeCamera == nullptr) ? defaultCamera : activeCamera;
	}

	void SetCamera(T * camera) {
		activeCamera = camera;
	}

	void SetDefaultCamera(T * defCam) {
		defaultCamera = defCam;
	}

	constexpr void Foreach(std::function<void(T *)> callback) {
		for(auto it = storage.begin(); it != nullptr; ++it) {
			callback(it.operator->());
		}
	}

	void Remove(T * obj) {
		storage.Remove(obj);
	}

	void SpawnPhysxActor(physx::PxActor * actor) {
		if(pxScene.Get() != nullptr) {
			pxScene->addActor(*actor);
		}
	}

	T* Create() {
		return storage.Emplace();
	}

	inline T * Create(const char * name) {
#if defined(NETCODE_DEBUG)
		T * t = Create();
		t->name = name;
		return t;
#else
		UNREFERENCED_PARAMETER(name);
		return Create();
#endif
	}
};
