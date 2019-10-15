#pragma once

#include "ConstantBuffer.hpp"

#include <DirectXMath.h>

/*
* Max bone count is 128, makes it a 8K cbuffer, use it sparingly
*/
__declspec(align(16)) struct BoneDataCb {
	constexpr static int id = 2;

	DirectX::XMFLOAT4X4A BindTransform[128];
};

struct AnimationComponent {

	void * pointer;

};
