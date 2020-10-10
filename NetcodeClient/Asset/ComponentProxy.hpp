#pragma once

#include "../GameObject.h"

template<typename T>
class ComponentProxy {
	GameObject * gameObj;
public:
	ComponentProxy(GameObject * obj) : gameObj{ obj } { }

	T * operator->() {
		if(gameObj->HasComponent<T>()) {
			return gameObj->GetComponent<T>();
		}
		return gameObj->AddComponent<T>();
	}
};
