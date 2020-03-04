#pragma once

#include "UIControl.h"

class UITextBox : public UIControl {
	int id;
	UIObject * backgroundObject;
	UIObject * buttonObject;
	UIObject * textObject;

	static int textBoxIdGenerator;
	static int selectedTextBoxId;
public:
	static void ClearSelection() {
		selectedTextBoxId = -1;
	}

	UITextBox(UIObject * backgroundObj, UIObject * buttonObj, UIObject *textObj) : id { textBoxIdGenerator++ },
		backgroundObject{ backgroundObj }, buttonObject{ buttonObj }, textObject{ textObj } {

		backgroundObject->AddComponent<Transform>();
		backgroundObject->AddComponent<UIElement>();
		
		buttonObject->AddComponent<Transform>();
		buttonObject->AddComponent<UIElement>();
		Button* btn = buttonObject->AddComponent<Button>();

		btn->onClick = [this]() -> void {
			selectedTextBoxId = id;
		};

		textObject->AddComponent<Transform>();
		textObject->AddComponent<Text>();
		textObject->AddComponent<TextBox>();
	}
};
