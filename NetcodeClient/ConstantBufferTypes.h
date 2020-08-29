#pragma once

#include <NetcodeFoundation/Math.h>

#include "Light.h"

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
	Netcode::Float4 ambientLightIntensity;
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

struct AnimationStaticConstants {
	constexpr static uint32_t MAX_ANIMATION_COUNT = 64;

	uint32_t numBones;
	uint32_t numAnimations;
	DirectX::XMUINT2 __padding_StaticConstants;
	uint32_t startIndices[MAX_ANIMATION_COUNT];
	int32_t parentIndices[BoneData::MAX_BONE_COUNT];
	Netcode::Float4x4 offsetMatrices[BoneData::MAX_BONE_COUNT];
};

struct AnimInstanceData {
	uint32_t numStates;
	DirectX::XMUINT3  __padding_AnimationInstance;
	float weights[8];
	float normalizedTimes[8];
	uint32_t animationIndices[8];
	uint32_t frameIndices[8 * 2];
};

struct AnimInstanceConstants {
	uint32_t numInstances;
	DirectX::XMUINT3 __padding_AnimationInstanceData;
	AnimInstanceData instances[32];
};

struct SsaoData {
	constexpr static int SAMPLE_COUNT = 14;

	Netcode::Float4 Offsets[SAMPLE_COUNT];
	float occlusionRadius;
	float occlusionFadeStart;
	float occlusionFadeEnd;
	float surfaceEpsilon;
	Netcode::Float4 weights[3];
	Netcode::Float2 invRenderTargetSize;
};


struct PerObjectData {
	Netcode::Float4x4 Model;
	Netcode::Float4x4 InvModel;
	int lightsCount;
	int lightsOffset;
};

struct DebugPhysxShapeData {
	Netcode::Float4x4 local;
	Netcode::Float4x4 offset;
};
