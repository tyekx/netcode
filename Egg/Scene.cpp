#include "Scene.h"

void Egg::Scene::Reset() {
	for(unsigned int i = 0; i < ObjectCount; ++i) {
		(Objects + i)->~GameObject();
	}
	std::free(Objects);
	Objects = reinterpret_cast<Egg::GameObject *>(std::malloc(MaxCount * sizeof(Egg::GameObject)));
}

Egg::GameObject * Egg::Scene::Next() {
	Egg::GameObject * ptr = Objects + ObjectCount;
	new (ptr) Egg::GameObject(ObjectCount++);
	EGG_DEBUG_ENGINE_NOTIFY_OBSERVER(SceneGameObjectAdded, ptr);
	return ptr;
}
