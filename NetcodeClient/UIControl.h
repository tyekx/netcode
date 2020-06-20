#pragma once

#include "UIObject.h"

class UIControl {
protected:
	void AlignText(Text  * text, Transform * transform, UIElement * element) {
		if(text == nullptr || transform == nullptr || element == nullptr || text->font == nullptr) {
			return;
		}

		Netcode::Float2 strSize;
		
		if(text->text.empty()) {
			strSize = text->font->MeasureString(L"a");
		} else {
			strSize = text->font->MeasureString(text->text.c_str());
		}

		switch(text->horizontalAlignment) {
			case HorizontalAnchor::LEFT:
				transform->position.x = element->padding.x;
				break;
			case HorizontalAnchor::CENTER:
				transform->position.x = (element->width - strSize.x) / 2.0f;
				break;
			case HorizontalAnchor::RIGHT:
				transform->position.x = (element->width - strSize.x) - element->padding.z;
				break;

		}

		switch(text->verticalAlignment) {
			case VerticalAnchor::TOP:
				transform->position.y = element->padding.y;
				break;
			case VerticalAnchor::MIDDLE:
				transform->position.y = (element->height - strSize.y) / 2.0f;
				break;
			case VerticalAnchor::BOTTOM:
				transform->position.y = (element->height - strSize.y) - element->padding.w;
				break;
		}
	}

public:
	virtual UIObject * GetRoot() = 0;

	virtual ~UIControl() = default;

	virtual void SetPosition(const Netcode::Float2 & pos) {
		UIObject * root = GetRoot();

		if(root->HasComponent<Transform>()) {
			Transform * bgTransform = root->GetComponent<Transform>();
			bgTransform->position.x = pos.x;
			bgTransform->position.y = pos.y; 
		}
	}
};
