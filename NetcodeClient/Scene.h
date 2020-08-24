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
	Netcode::PxPtr<physx::PxMaterial> controllerMaterial;
	Netcode::BulkVector<T, 512> storage;
	T * cameraRef;

public:
	virtual ~Scene() = default;

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
		return cameraRef;
	}

	void SetCamera(T * camera) {
		cameraRef = camera;
	}

	void Foreach(std::function<void(T *)> callback) {
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

	inline T * Create(const char * debugName) {
#if defined(NETCODE_DEBUG)
		T * t = Create();
		t->debugName = debugName;
		return t;
#else
		UNREFERENCED_PARAMETER(debugName);
		return Create();
#endif
	}
};
