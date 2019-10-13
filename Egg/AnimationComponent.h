#pragma once

#include "ConstantBuffer.hpp"

#include "Math/Float4x4.h"

/*
* Max bone count is 128, makes it a 8K cbuffer, use it sparingly
*/
__declspec(align(16)) struct BoneDataCb {
	constexpr static int id = 2;

	Egg::Math::Float4x4 BindTransform[128];
};

struct AnimationComponent {

	void * pointer;

};
