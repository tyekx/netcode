#pragma once

#include "Snippets.h"
#include "GameObject.h"

class RemotePlayerPrefab {
	RemotePlayerPrefab() = delete;
	~RemotePlayerPrefab() = delete;
public:

	static void Create(Egg::Asset::Model* modelAsset, GameObject* gameObject) {

		Animation* anim = gameObject->AddComponent<Animation>();

		CreateYbotAnimationComponent(modelAsset, anim);

	}

};
