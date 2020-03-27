#pragma once

#include "UIControl.h"
#include <Netcode/Input.h>

class UITextBox : public UIControl {
	UIObject * backgroundObject;
	UIObject * textObject;
public:

	UITextBox() = default;

	virtual UIObject * GetRoot() override {
		return backgroundObject;
	}

	void SetPlaceholder(std::wstring placeholder) {
		TextBox * textBoxComponent = textObject->AddComponent<TextBox>();
		textBoxComponent->placeholder = std::move(placeholder);
	}

	std::wstring GetValue() {
		Text * textComponent = textObject->GetComponent<Text>();
		return textComponent->text;
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

	void SetSize(const DirectX::XMFLOAT2 & size) {
		UIElement * bgElem = backgroundObject->AddComponent<UIElement>();
		bgElem->width = size.x;
		bgElem->height = size.y;
	}

	void SetBackgroundImage(Egg::ResourceViewsRef resourceView, const DirectX::XMUINT2 & textureSize) {
		Sprite * sprite = nullptr;
		if(!backgroundObject->HasComponent<Sprite>()) {
			sprite = backgroundObject->AddComponent<Sprite>();
		} else {
			sprite = backgroundObject->GetComponent<Sprite>();
		}

		sprite->texture = resourceView;
		sprite->textureSize = textureSize;
		sprite->diffuseColor = DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f };
	}

	void SetPasswordFlag() {
		TextBox * textBoxComponent = textObject->GetComponent<TextBox>();
		textBoxComponent->isPassword = true;
	}

	UITextBox(UIObject * backgroundObj, UIObject *textObj) :
		backgroundObject{ backgroundObj }, textObject{ textObj } {

		textObject->Parent(backgroundObject);

		backgroundObject->AddComponent<Transform>();
		UIElement * bgElem = backgroundObject->AddComponent<UIElement>();
		SetSize(DirectX::XMFLOAT2{ 390.0f, 48.0f });
		bgElem->padding = DirectX::XMFLOAT4{ 15.0f, 0.0f, 15.0f, 0.0f };

		Button* btn = backgroundObject->AddComponent<Button>();

		textObject->AddComponent<Transform>();
		UIElement * uiElem = textObject->AddComponent<UIElement>();
		Text * textComponent = textObject->AddComponent<Text>();
		textComponent->text = L"";
		textComponent->color = DirectX::XMFLOAT4{ 1.0f, 1.0f, 1.0f, 1.0f };
		textComponent->verticalAlignment = VerticalAnchor::MIDDLE;
		textComponent->horizontalAlignment = HorizontalAnchor::LEFT;

		TextBox* textBoxComponent = textObject->AddComponent<TextBox>();
		textBoxComponent->placeholder = L"empty";
		textBoxComponent->placeholderColor = DirectX::XMFLOAT4{ 0.4f, 0.4f, 0.4f, 0.7f };
		textBoxComponent->maxCharacters = 24;
		textBoxComponent->caretPosition = 0;
		textBoxComponent->selectionEndIndex = -1;

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

		textBoxComponent->keyPressedToken = Egg::Input::OnKeyPressed += [textBoxComponent, textComponent](uint32_t key, uint32_t modifiers) -> void {
			if(textBoxComponent->id == TextBox::selectedId) {
				Log::Debug("[TextBox({0})] key pressed: {1}", textBoxComponent->id, key);
				if(key == VK_BACK) {
					if(!textComponent->text.empty()) {
						Log::Debug("[TextBox({0})] char erased: {1}", textBoxComponent->id, *textComponent->text.crbegin());
						textComponent->text.erase(textComponent->text.begin() + textBoxComponent->caretPosition - 1);
						textBoxComponent->caretPosition -= 1;
					}
				}

				if(key == VK_LEFT) {
					textBoxComponent->caretPosition = std::max<int>(textBoxComponent->caretPosition - 1, 0);
				}

				if(key == VK_RIGHT) {
					textBoxComponent->caretPosition = std::min<int>(textBoxComponent->caretPosition + 1, static_cast<int>(textBoxComponent->maxCharacters));
				}

				if((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9') || (key == ' ')) {
					if(textComponent->text.size() >= textBoxComponent->maxCharacters) {
						Log::Debug("[TextBox({0})] full, keypress ignored: {1}", textBoxComponent->id, key);
						return;
					}
					if(((modifiers & Egg::KeyModifiers::SHIFT_CAPS_LOCK) != Egg::KeyModifiers::SHIFT_CAPS_LOCK) && (
						((modifiers & Egg::KeyModifiers::SHIFT) == Egg::KeyModifiers::SHIFT) ||
						((modifiers & Egg::KeyModifiers::CAPS_LOCK) == Egg::KeyModifiers::CAPS_LOCK))) {
					} else {
						key = std::tolower(key);
					}

					textComponent->text.insert(textComponent->text.begin() + textBoxComponent->caretPosition, static_cast<wchar_t>(key));
					textBoxComponent->caretPosition += 1;

					Log::Debug("[TextBox({0})] char added: {1}", textBoxComponent->id, key);
				}
			}
		};
	}
};
