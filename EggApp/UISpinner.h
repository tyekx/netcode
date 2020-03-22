#pragma once

#include "UIControl.h"

class UISpinner : public UIControl {
	UIObject * root;

public:
	virtual UIObject * GetRoot() override {
		return root;
	}

	UISpinner() = default;

	void SetPosition(const DirectX::XMFLOAT2 & pos) {
		Transform * lTrans = root->GetComponent<Transform>();
		UIElement * lElem = root->GetComponent<UIElement>();

		lTrans->position = DirectX::XMFLOAT3{ pos.x + lElem->width / 2.0f, pos.y + lElem->height / 2.0f, 0.0f };
	}

	void SetSize(const DirectX::XMFLOAT2 & size) {
		Transform * lTrans = root->GetComponent<Transform>();
		UIElement * lElem = root->GetComponent<UIElement>();
		Sprite * sprite = root->GetComponent<Sprite>();

		lElem->width = size.x;
		lElem->height = size.y;
		lTrans->position = DirectX::XMFLOAT3{ lElem->width / 2.0f, lElem->height / 2.0f, 0.0f };
	}

	void SetIcon(Egg::ResourceViewsRef resourceView, const DirectX::XMUINT2 & textureSize) {
		UIElement * lElem = root->GetComponent<UIElement>();
		Transform * lTrans = root->GetComponent<Transform>();
		Sprite * sprite = root->GetComponent<Sprite>();

		DirectX::XMFLOAT2 texSize = DirectX::XMFLOAT2{
			static_cast<float>(textureSize.x),
			static_cast<float>(textureSize.y)
		};

		lElem->width = texSize.x;
		lElem->height = texSize.y;
		lElem->origin = DirectX::XMFLOAT2{ texSize.x / 2.0f  , texSize.y / 2.0f };
		lTrans->position = DirectX::XMFLOAT3{ lElem->width / 2.0f, lElem->height / 2.0f, 0.0f };
		sprite->texture = resourceView;
		sprite->textureSize = textureSize;
	}

	UISpinner(UIObject* obj) : root { obj } {
		Transform * lTrans = root->AddComponent<Transform>();
		UIElement * lElem = root->AddComponent<UIElement>();
		Sprite * sprite = root->AddComponent<Sprite>();
		sprite->diffuseColor = DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f };

		SpriteAnimation * spriteAnim = root->AddComponent<SpriteAnimation>();
		spriteAnim->onUpdate = [lElem](UIObject * object, float dt) -> void {
			lElem->rotationZ += 10.0f * dt;
		};
	}
};
