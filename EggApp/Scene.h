#pragma once

#include "GameObject.h"
#include <list>
#include <memory>
#include <Egg/BulkVector.hpp>

/*
base class
*/
template<typename T>
class Scene {
protected:
	Egg::BulkVector<T, 512> storage;
	physx::PxScene * pxScene;
	T * cameraRef;

public:

	physx::PxScene * GetPhysXScene() const {
		return pxScene;
	}

	void SetPhysXScene(physx::PxScene * scene) {
		pxScene = scene;
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
		if(pxScene) {
			pxScene->addActor(*actor);
		}
	}

	T* Create() {
		return storage.Emplace();
	}
};
