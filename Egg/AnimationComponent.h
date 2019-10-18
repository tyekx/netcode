#pragma once

#include "ConstantBuffer.hpp"

#include <DirectXMath.h>

/*
* Max bone count is 128, makes it a 16K cbuffer, use it sparingly
*/
__declspec(align(16)) struct BoneDataCb {
	constexpr static int id = 2;
	constexpr static int MAX_BONE_COUNT = 128;

	DirectX::XMFLOAT4X4A BindTransform[MAX_BONE_COUNT];
	DirectX::XMFLOAT4X4A ToRootTransform[MAX_BONE_COUNT];
};

struct AnimationComponent {

	void * pointer;

};
