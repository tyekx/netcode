#pragma once

#include "Scene.h"

namespace Egg {

	class Scene;

	struct ISceneObserver {
		virtual ~ISceneObserver() = default;
		virtual void SceneGameObjectAdded(Egg::GameObject * gameObject) = 0;
	/* not implemented yet	virtual void SceneGameObjectRemoved(Egg::GameObject * gameObject) = 0; */
	};

}
