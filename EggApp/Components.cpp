#include "Components.h"

int TextBox::idGenerator{ 0 };

int TextBox::selectedId{ -1 };

TextBox::TextBox() : id{ idGenerator++ }, keyPressedToken{ 0 }, contentChanged{}, isPassword{ false } {

}

TextBox::~TextBox() {
	Egg::Input::OnKeyPressed -= keyPressedToken;
}

