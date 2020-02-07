#pragma once

#include <DirectXMath.h>

struct PerFrameData {
	DirectX::XMFLOAT4X4A ViewProj;
	DirectX::XMFLOAT4X4A ViewProjInv;
	DirectX::XMFLOAT4X4A View;
	DirectX::XMFLOAT4X4A Proj;
	DirectX::XMFLOAT4X4A ViewInv;
	DirectX::XMFLOAT4X4A ProjInv;
	DirectX::XMFLOAT4X4A ProjTex;
	DirectX::XMFLOAT4X4A RayDir;
	DirectX::XMFLOAT4A eyePos;
	float nearZ;
	float farZ;
	float aspectRatio;
	float fov;
};

struct BoneData {
	constexpr static int MAX_BONE_COUNT = 128;
	DirectX::XMFLOAT4X4A BindTransform[MAX_BONE_COUNT];
	DirectX::XMFLOAT4X4A ToRootTransform[MAX_BONE_COUNT];
};

struct BoneVisibilityData {
	uint32_t BoneVisibility[BoneData::MAX_BONE_COUNT];
};

struct PerObjectData {
	DirectX::XMFLOAT4X4A Model;
	DirectX::XMFLOAT4X4A InvModel;
};
