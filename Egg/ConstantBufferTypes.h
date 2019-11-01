#pragma once

#include <DirectXMath.h>
#include "Light.h"

#define CB_TYPE __declspec(align(256))

CB_TYPE struct PerFrameCb {
	constexpr static int id = 0;

	DirectX::XMFLOAT4X4A ViewProj;
	DirectX::XMFLOAT4X4A ViewProjInv;
	DirectX::XMFLOAT4X4A View;
	DirectX::XMFLOAT4X4A Proj;
	DirectX::XMFLOAT3A eyePos;
	Egg::Light Light;
};

CB_TYPE struct PerObjectCb {
	constexpr static int id = 1;

	DirectX::XMFLOAT4X4A Model;
	DirectX::XMFLOAT4X4A InvModel;
};

CB_TYPE struct BoneDataCb {
	constexpr static int id = 2;
	constexpr static int MAX_BONE_COUNT = 128;

	DirectX::XMFLOAT4X4A BindTransform[MAX_BONE_COUNT];
	DirectX::XMFLOAT4X4A ToRootTransform[MAX_BONE_COUNT];
};

CB_TYPE struct PerMeshCb {
	constexpr static int id = 3;

	DirectX::XMFLOAT4A diffuseColor;
	DirectX::XMFLOAT3 fresnelR0;
	float shininess;
};

int GetConstantBufferIdFromName(const char * name);
