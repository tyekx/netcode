#pragma once

#include "GameObject.h"
#include <list>
#include <memory>
#include <Egg/BulkAllocator.hpp>

/*
base class
*/
class Scene {
protected:
	static void Swap(GameObject * lhs, GameObject * rhs) {
		std::swap(*lhs, *rhs);
	}

	Egg::BulkAllocator<GameObject, 512, true> storage;
	physx::PxScene * pxScene;
	GameObject * cameraRef;

public:

	physx::PxScene * GetPhysXScene() const {
		return pxScene;
	}

	void SetPhysXScene(physx::PxScene * scene) {
		pxScene = scene;
	}

	GameObject * GetCamera() const {
		return cameraRef;
	}

	void SetCamera(GameObject * camera) {
		cameraRef = camera;
	}

	void Foreach(std::function<void(GameObject *)> callback) {
		for(auto it = storage.begin(); it != nullptr; ++it) {
			callback(it.operator->());
		}
	}

	void Remove(GameObject * obj) {
		storage.Deallocate(obj);
	}

	void SpawnPhysxActor(physx::PxActor * actor) {
		if(pxScene) {
			pxScene->addActor(*actor);
		}
	}

	void Spawn(GameObject * obj) {
		if(!obj->IsDeletable()) {
			if(obj->HasComponent<Collider>()) {
				SpawnPhysxActor(obj->GetComponent<Collider>()->actorRef);
			}
			obj->Spawn();
		}
	}

	GameObject* Create() {
		return storage.Allocate();
	}
};
