#pragma once

#include <Egg/Math/Float4x4.h>

using namespace Egg::Math;

__declspec(align(16)) struct PerFrameCb {

	Float4x4 viewMatrix;
	Float4x4 projMatrix;

};
