#pragma once

#include "PlayerBehavior.h"
#include "GameObject.h"

class LocalPlayerPrefab {
	LocalPlayerPrefab() = delete;
	~LocalPlayerPrefab() = delete;
public:
	static void Create(GameObject * gameObject) {
		auto playerBehav = std::make_unique<PlayerBehavior>();
		
	}
};
