#pragma once

#include <DirectXMath.h>

struct PerFrameData {
	Netcode::Float4x4 ViewProj;
	Netcode::Float4x4 ViewProjInv;
	Netcode::Float4x4 View;
	Netcode::Float4x4 Proj;
	Netcode::Float4x4 ViewInv;
	Netcode::Float4x4 ProjInv;
	Netcode::Float4x4 ProjTex;
	Netcode::Float4x4 RayDir;
	Netcode::Float4 eyePos;
	float nearZ;
	float farZ;
	float aspectRatio;
	float fov;
};

struct BoneData {
	constexpr static int MAX_BONE_COUNT = 128;
	Netcode::Float4x4 BindTransform[MAX_BONE_COUNT];
	Netcode::Float4x4 ToRootTransform[MAX_BONE_COUNT];
};

struct BoneVisibilityData {
	uint32_t BoneVisibility[BoneData::MAX_BONE_COUNT];
};

struct PerObjectData {
	Netcode::Float4x4 Model;
	Netcode::Float4x4 InvModel;
};

struct ColliderData {
	Netcode::Float4x4 LocalTransform;
	Netcode::Float4 Color;
	uint32_t BoneReference;
};
