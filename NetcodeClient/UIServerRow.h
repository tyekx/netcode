#pragma once

#include "UIControl.h"

class UIServerRow : public UIControl {
	UIObject * rootObject;
	UIObject * ipObject;
	UIObject * slotsObject;
	UIObject * creatorObject;
public:
	virtual UIObject * GetRoot() override {
		return rootObject;
	}

	void ServerRowAlignText(Text * text, float leftOffset, Transform * transform, UIElement * element) {
		if(text == nullptr || transform == nullptr || element == nullptr || text->font == nullptr) {
			return;
		}

		DirectX::XMFLOAT2 strSize;

		if(text->text.empty()) {
			strSize = text->font->MeasureString(L"a");
		} else {
			strSize = text->font->MeasureString(text->text.c_str());
		}

		switch(text->horizontalAlignment) {
			case HorizontalAnchor::LEFT:
				transform->position.x = leftOffset + element->padding.x;
				break;
			case HorizontalAnchor::CENTER:
				transform->position.x = leftOffset + (element->width - strSize.x) / 2.0f;
				break;
			case HorizontalAnchor::RIGHT:
				transform->position.x = leftOffset + (element->width - strSize.x) - element->padding.z;
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

	void SetFont(Netcode::SpriteFontRef spriteFont) {
		Text * t1 = ipObject->GetComponent<Text>();
		Text * t2 = slotsObject->GetComponent<Text>();
		Text * t3 = creatorObject->GetComponent<Text>();

		t1->font = spriteFont;
		t2->font = spriteFont;
		t3->font = spriteFont;

		Transform * tr1 = ipObject->GetComponent<Transform>();
		Transform * tr2 = slotsObject->GetComponent<Transform>();
		Transform * tr3 = creatorObject->GetComponent<Transform>();

		ServerRowAlignText(t1, 0.0f, tr1, ipObject->GetComponent<UIElement>());
		ServerRowAlignText(t2, 652.0f, tr2, slotsObject->GetComponent<UIElement>());
		ServerRowAlignText(t3, 452.0f, tr3, creatorObject->GetComponent<UIElement>());
	}

	void SetOwnerName(std::wstring name) {
		Text * t3 = creatorObject->GetComponent<Text>();
		Transform * tr3 = creatorObject->GetComponent<Transform>();
		t3->text = std::move(name);
		ServerRowAlignText(t3, 452.0f, tr3, creatorObject->GetComponent<UIElement>());
	}

	void SetSlots(std::wstring t) {
		Text * t2 = slotsObject->GetComponent<Text>();
		Transform * tr2 = slotsObject->GetComponent<Transform>();
		t2->text = std::move(t);
		ServerRowAlignText(t2, 652.0f, tr2, slotsObject->GetComponent<UIElement>());
	}

	void SetBackground(Netcode::ResourceViewsRef res, const Netcode::UInt2 & texSize) {
		Sprite * sprite = rootObject->GetComponent<Sprite>();
		sprite->texture = std::move(res);
		sprite->textureSize = texSize;
	}

	UIServerRow() = default;

	UIServerRow(UIObject * root, UIObject * name, UIObject * slots, UIObject * creator) :
		rootObject{ root }, ipObject{ name }, slotsObject{ slots }, creatorObject{ creator } {

			{
				Transform * transform = rootObject->AddComponent<Transform>();
				UIElement * uiElem = rootObject->AddComponent<UIElement>();
				Sprite * sprite = rootObject->AddComponent<Sprite>();
				rootObject->AddComponent<Button>();

				transform->position = Netcode::Float3{ 8.0f, 0.0f, 0.0f };
				uiElem->width = 752.0f;
				uiElem->height = 48.0f;
				sprite->hoverColor = Netcode::Float4{ 0.5f, 0.5f, 0.5f, 1.0f };
				sprite->diffuseColor = Netcode::Float4::One;
			}

			{
				Transform * transform = ipObject->AddComponent<Transform>();
				transform->position = Netcode::Float3::Zero;
				UIElement * uiElem = ipObject->AddComponent<UIElement>();
				uiElem->width = 452.0f;
				uiElem->height = 42.0f;
				Text * text = ipObject->AddComponent<Text>();
				text->text = L"2a01:36d:120:4ee3:ad5b:fed3:c49b:344e";
				text->horizontalAlignment = HorizontalAnchor::LEFT;
				text->color = Netcode::Float4::One;
				text->verticalAlignment = VerticalAnchor::MIDDLE;
			}

			{
				Transform * transform = creatorObject->AddComponent<Transform>();
				transform->position = Netcode::Float3::Zero;
				UIElement * uiElem = creatorObject->AddComponent<UIElement>();

				uiElem->width = 200.0f;
				uiElem->height = 42.0f;

				Text * text = creatorObject->AddComponent<Text>();
				text->text = L"Tyekx";
				text->horizontalAlignment = HorizontalAnchor::RIGHT;
				text->color = Netcode::Float4::One;
				text->verticalAlignment = VerticalAnchor::MIDDLE;
			}

			{
				Transform * transform = slotsObject->AddComponent<Transform>();
				transform->position = Netcode::Float3::Zero;
				UIElement * uiElem = slotsObject->AddComponent<UIElement>();
				uiElem->width = 100.0f;
				uiElem->height = 42.0f;
				Text * text = slotsObject->AddComponent<Text>();
				text->text = L"0/12";
				text->horizontalAlignment = HorizontalAnchor::RIGHT;
				text->color = Netcode::Float4::One;
				text->verticalAlignment = VerticalAnchor::MIDDLE;
			}

	}
};
