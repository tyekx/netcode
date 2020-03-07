#pragma once

#include "UIControl.h"
#include <Egg/Input.h>

class UITextBox : public UIControl {
	UIObject * backgroundObject;
	UIObject * buttonObject;
	UIObject * textObject;

	void AlignText() {
		Text * textComponent = textObject->GetComponent<Text>();
		Transform * textTransform = textObject->GetComponent<Transform>();
		UIElement * bgElem = backgroundObject->GetComponent<UIElement>();

		if(textComponent->font != nullptr) {
			DirectX::XMFLOAT2 strSize = textComponent->font->MeasureString(textComponent->text.c_str());
			textTransform->position.x = (bgElem->width - strSize.x) / 2.0f;
			textTransform->position.y = (bgElem->height - strSize.y) / 2.0f;
		}
	}
public:

	virtual UIObject * GetRoot() override {
		return backgroundObject;
	}

	void SetText(std::wstring text) {
		Text * textComponent = textObject->GetComponent<Text>();
		textComponent->text = std::move(text);
		AlignText();
	}

	void SetFont(Egg::SpriteFontRef spriteFont) {
		Text * textComponent = textObject->GetComponent<Text>();
		textComponent->font = std::move(spriteFont);
		AlignText();
	}

	UITextBox(UIObject * backgroundObj, UIObject * buttonObj, UIObject *textObj) :
		backgroundObject{ backgroundObj }, buttonObject{ buttonObj }, textObject{ textObj } {

		backgroundObject->AddComponent<Transform>();
		UIElement * bgElem = backgroundObject->AddComponent<UIElement>();
		bgElem->width = 360.0f;
		bgElem->height = 64.0f;

		buttonObject->AddComponent<Transform>();
		UIElement * btnElem = buttonObject->AddComponent<UIElement>();
		btnElem->width = 360.0f;
		btnElem->height = 64.0f;
		
		Button* btn = buttonObject->AddComponent<Button>();

		textObject->AddComponent<Transform>();
		textObject->AddComponent<UIElement>();
		Text * textComponent = textObject->AddComponent<Text>();
		textComponent->text = L"H";
		TextBox* textBoxComponent = textObject->AddComponent<TextBox>();

		btn->onMouseEnter = [textBoxComponent]() ->void {
			Log::Debug("[TextBox({0})] onMouseEnter", textBoxComponent->id);
		};

		btn->onMouseLeave = [textBoxComponent]() ->void {
			Log::Debug("[TextBox({0})] onMouseLeave", textBoxComponent->id);
		};

		btn->onClick = [textBoxComponent]() -> void {
			TextBox::selectedId = textBoxComponent->id;
			Log::Debug("[TextBox({0})] onClick, now selected", textBoxComponent->id);
		};

		textBoxComponent->keyPressedToken = Egg::Input::OnKeyPressed += [textBoxComponent, textComponent](uint32_t key) -> void {
			if(textBoxComponent->id == TextBox::selectedId) {
				Log::Debug("[TextBox({0})] key pressed: {1}", textBoxComponent->id, key);
				if(key == VK_BACK) {
					if(!textComponent->text.empty()) {
						Log::Debug("[TextBox({0})] char erased: {1}", textBoxComponent->id, *textComponent->text.crbegin());
						textComponent->text.pop_back();
					}
				}
				if((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) {
					textComponent->text += static_cast<wchar_t>(key);
					Log::Debug("[TextBox({0})] char added: {1}", textBoxComponent->id, key);
				}
			}
		};
	}
};
