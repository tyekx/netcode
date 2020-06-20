#pragma once

#include "UIControl.h"

class UISpinner : public UIControl {
	UIObject * root;

public:
	virtual UIObject * GetRoot() override {
		return root;
	}

	UISpinner() = default;

	virtual void SetPosition(const Netcode::Float2 & pos) override {
		Transform * lTrans = root->GetComponent<Transform>();
		UIElement * lElem = root->GetComponent<UIElement>();

		lTrans->position = Netcode::Float3{ pos.x + lElem->width / 2.0f, pos.y + lElem->height / 2.0f, 0.0f };
	}

	void SetSize(const Netcode::Float2 & size) {
		Transform * lTrans = root->GetComponent<Transform>();
		UIElement * lElem = root->GetComponent<UIElement>();

		lElem->width = size.x;
		lElem->height = size.y;
		lTrans->position = Netcode::Float3{ lElem->width / 2.0f, lElem->height / 2.0f, 0.0f };
	}

	void SetIcon(Netcode::ResourceViewsRef resourceView, const Netcode::UInt2 & textureSize) {
		UIElement * lElem = root->GetComponent<UIElement>();
		Transform * lTrans = root->GetComponent<Transform>();
		Sprite * sprite = root->GetComponent<Sprite>();

		Netcode::Float2 texSize {
			static_cast<float>(textureSize.x),
			static_cast<float>(textureSize.y)
		};

		lElem->width = texSize.x;
		lElem->height = texSize.y;
		lElem->origin = Netcode::Float2{ texSize.x / 2.0f  , texSize.y / 2.0f };
		lTrans->position = Netcode::Float3{ lElem->width / 2.0f, lElem->height / 2.0f, 0.0f };
		sprite->texture = resourceView;
		sprite->textureSize = textureSize;
	}

	UISpinner(UIObject* obj) : root { obj } {
		root->AddComponent<Transform>();
		UIElement * lElem = root->AddComponent<UIElement>();
		Sprite * sprite = root->AddComponent<Sprite>();
		sprite->diffuseColor = Netcode::Float4::One;

		UIScript * uiScript = root->AddComponent<UIScript>();
		uiScript->onUpdate = [lElem](UIObject * object, float dt) -> void {
			lElem->rotationZ += 10.0f * dt;
		};
	}
};
