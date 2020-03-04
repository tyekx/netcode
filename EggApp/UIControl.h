#pragma once

#include "UIObject.h"

class UIControl {
public:
	virtual UIObject * GetRoot() = 0;
};
