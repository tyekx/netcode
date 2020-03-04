#pragma once

#include "UIControl.h"

class UITextBox : public UIControl {
	UIObject * backgroundObject;
	UIObject * buttonObject;
	UIObject * textObject;

	static UITextBox * selectedTextBox;
public:
	static void ClearSelection() {
		selectedTextBox = nullptr;
	}

	UITextBox(UIObject * backgroundObj, UIObject * buttonObj, UIObject *textObj) : 
		backgroundObject{ backgroundObj }, buttonObject{ buttonObj }, textObject{ textObj } {

		backgroundObject->AddComponent<Transform>();
		backgroundObject->AddComponent<UIElement>();
		
		buttonObject->AddComponent<UIElement>();
		Button* btn = buttonObject->AddComponent<Button>();

		btn->onClick = [&]() -> void {
			selectedTextBox = this;
		};

	}

};
