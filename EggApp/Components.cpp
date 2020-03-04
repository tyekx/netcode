#include "Components.h"

int TextBox::idGenerator{ 0 };

int TextBox::selectedId{ -1 };

TextBox::TextBox() : id{ idGenerator++ }, contentChanged{} {

}

