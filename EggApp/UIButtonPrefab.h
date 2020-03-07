#pragma once

#include <Egg/HandleTypes.h>
#include "UIObject.h"
#include "UIControl.h"

class UIButtonPrefab : public UIControl {
	UIObject * backgroundObject;
	UIObject * textObject;

public:
	void SetSize(const DirectX::XMFLOAT2 & size) {
		UIElement * bgElem = backgroundObject->GetComponent<UIElement>();
		bgElem->width = size.x;
		bgElem->height = size.y;
		AlignText(textObject->GetComponent<Text>(), textObject->GetComponent<Transform>(), bgElem);
	}

	void SetBackgroundImage(Egg::ResourceViewsRef resourceView, const DirectX::XMUINT2 & dimensions) {
		Sprite * sprite = backgroundObject->AddComponent<Sprite>();
		sprite->texture = std::move(resourceView);
		sprite->textureSize = dimensions;
	}

	void SetText(std::wstring text) {
		Text * textComponent = textObject->GetComponent<Text>();
		textComponent->text = std::move(text);
		AlignText(textComponent, textObject->GetComponent<Transform>(), backgroundObject->GetComponent<UIElement>());
	}

	void SetFont(Egg::SpriteFontRef spriteFont) {
		Text * textComponent = textObject->GetComponent<Text>();
		textComponent->font = std::move(spriteFont);
		AlignText(textComponent, textObject->GetComponent<Transform>(), backgroundObject->GetComponent<UIElement>());
	}

	void SetZIndex(float z) {
		Transform * bgTransform = backgroundObject->GetComponent<Transform>();
		bgTransform->position.z = z;
	}

	void OnClick(std::function<void()> onClick) {
		Button * btn = backgroundObject->GetComponent<Button>();
		btn->onClick = onClick;
	}

	void OnMouseEnter(std::function<void()> onMouseEnter) {
		Button * btn = backgroundObject->GetComponent<Button>();
		btn->onMouseEnter = onMouseEnter;
	}

	void OnMouseLeave(std::function<void()> onMouseLeave) {
		Button * btn = backgroundObject->GetComponent<Button>();
		btn->onMouseLeave = onMouseLeave;
	}

	virtual UIObject * GetRoot() override {
		return backgroundObject;
	}

	UIButtonPrefab(UIObject * background, UIObject * textObject) : backgroundObject{ background }, textObject{ textObject } {
		textObject->Parent(background);

		UIElement * bgElem = background->AddComponent<UIElement>();
		bgElem->width = 256.0f;
		bgElem->height = 64.0f;

		Button * bgBtn = background->AddComponent<Button>();

		Sprite * sprite = background->AddComponent<Sprite>();
		sprite->textureSize = DirectX::XMUINT2{ 0,0 };
		sprite->texture = nullptr;
		sprite->diffuseColor = DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f };
		sprite->hoverColor = DirectX::XMFLOAT4{ 0.5f, 0.5f, 0.5f, 1.0f };

		Transform * bgTransform = background->AddComponent<Transform>();
		Transform * textTransform = textObject->AddComponent<Transform>();
		UIElement * textElem = textObject->AddComponent<UIElement>();

		Text * txt = textObject->AddComponent<Text>();
		txt->horizontalAlignment = HorizontalAnchor::CENTER;
		txt->verticalAlignment = VerticalAnchor::MIDDLE;
		txt->color = DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f };
	}
};
