#include "GameObject.h"

void Egg::GameObject::SetName(const std::string & name) {
	Name = name;
	EGG_DEBUG_ENGINE_NOTIFY_OBSERVER(GameObjectNameChanged, this);
}

Egg::GameObject::~GameObject() {
	CompositeObjectDestructor<COMPONENTS_T>::Invoke(Signature, Data);
	EGG_DEBUG_ENGINE_NOTIFY_OBSERVER(GameObjectDestroyed, this);
}

void Egg::GameObject::SetOwner(GameObject * o) {
	Owner = o;
	EGG_DEBUG_ENGINE_NOTIFY_OBSERVER(GameObjectOwnerChanged, this);
}
