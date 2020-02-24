#pragma once

#include "GameObject.h"
#include <list>
#include <memory>
#include <Egg/BulkVector.hpp>

/*
base class
*/
class Scene {
protected:
	static void Swap(GameObject * lhs, GameObject * rhs) {
		std::swap(*lhs, *rhs);
	}

	Egg::BulkVector<GameObject, 512> storage;
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
		storage.Remove(obj);
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
		return storage.Emplace();
	}
};
