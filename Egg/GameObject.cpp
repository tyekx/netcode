#include "GameObject.h"

void Egg::GameObject::SetName(const std::string & name) {
	Name = name;
}

Egg::GameObject::~GameObject() {
	CompositeObjectDestructor<COMPONENTS_T>::Invoke(Signature, Data);
}

void Egg::GameObject::SetOwner(GameObject * o) {
	Owner = o;
}
