#pragma once

#include "GameObject.h"

namespace Egg {

	class GameObject;

	struct IGameObjectObserver {
		virtual ~IGameObjectObserver() = default;
		virtual void GameObjectDestroyed(GameObject * obj) = 0;
		virtual void GameObjectOwnerChanged(GameObject * obj) = 0;
		virtual void GameObjectSignatureChanged(GameObject * obj) = 0;
		virtual void GameObjectNameChanged(GameObject * obj) = 0;
	};

}

