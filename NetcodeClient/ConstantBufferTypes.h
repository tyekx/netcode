#pragma once

#include <DirectXMath.h>
#include "Light.h"

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

struct AnimationStaticConstants {
	constexpr static uint32_t MAX_ANIMATION_COUNT = 64;

	uint32_t numBones;
	uint32_t numAnimations;
	DirectX::XMUINT2 __padding_StaticConstants;
	uint32_t startIndices[MAX_ANIMATION_COUNT];
	int32_t parentIndices[BoneData::MAX_BONE_COUNT];
	DirectX::XMFLOAT4X4 offsetMatrices[BoneData::MAX_BONE_COUNT];
}

struct SsaoData {
	constexpr static int SAMPLE_COUNT = 14;

	DirectX::XMFLOAT4A Offsets[SAMPLE_COUNT];
	float occlusionRadius;
	float occlusionFadeStart;
	float occlusionFadeEnd;
	float surfaceEpsilon;
	DirectX::XMFLOAT4A weights[3];
	DirectX::XMFLOAT2 invRenderTargetSize;
};


struct PerObjectData {
	DirectX::XMFLOAT4X4A Model;
	DirectX::XMFLOAT4X4A InvModel;
};

struct MaterialData {
	DirectX::XMFLOAT4A diffuseColor;
	DirectX::XMFLOAT3 fresnelR0;
	float shininess;
};

struct DebugPhysxShapeData {
	DirectX::XMFLOAT4X4A local;
	DirectX::XMFLOAT4X4A offset;
};
