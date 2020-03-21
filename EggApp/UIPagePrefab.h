#pragma once

#include "UIObject.h"
#include "UIControl.h"

class UIPagePrefab : public UIControl {
	UIObject * rootObject;
public:
	UIPagePrefab() = default;

	UIPagePrefab(UIObject * object) : rootObject{ object } {
		object->AddComponent<Transform>();
		object->AddComponent<UIElement>();
	}

	void SetSize(float width, float height) {
		auto * uiElem = rootObject->GetComponent<UIElement>();
		uiElem->width = width;
		uiElem->height = height;
	}

	void AddControl(UIControl & ctrl) {
		ctrl.GetRoot()->Parent(rootObject);
	}

	virtual UIObject * GetRoot() override {
		return rootObject;
	}

	void Show() {
		rootObject->SetActivityFlag(true);
	}

	void Hide() {
		rootObject->SetActivityFlag(false);
	}
};
