#include "RootSignatures.hlsli"
#include "Lighting.hlsli"

struct IAOutput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float3 weights : WEIGHTS;
	int4 boneIds : BONEIDS;
};

struct VSOutput {
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

cbuffer MaterialCb : register(b0)
{
	Material material;
}

cbuffer PerObjectCb : register(b1) {
	float4x4 modelMat;
	float4x4 invModelMat;
}

cbuffer BoneDataCb : register(b2) {
	float4x4 boneTransforms[128];
	float4x4 toRootTransforms[128];
}

cbuffer PerFrameCb : register(b3) {
	float4x4 viewProj;
	float4 eyePos;
	Light light;
}


[RootSignature(AvatarRootSignature)]
VSOutput main(IAOutput iao)
{
	VSOutput vso;
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = iao.weights[0];
	weights[1] = iao.weights[1];
	weights[2] = iao.weights[2];
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	
	float3 posL = float3(0, 0, 0);
	float3 normalL = float3(0, 0, 0);

	for(int i = 0; i < 4; ++i) {
		posL += weights[i] * (mul(float4(iao.position, 1), boneTransforms[iao.boneIds[i]]).xyz);
		normalL += weights[i] * (mul(iao.normal, (float3x3)boneTransforms[iao.boneIds[i]]));
	}
	iao.normal = normalL;

	float4 posW = mul(float4(posL, 1), modelMat);

	vso.position = mul(posW, viewProj);
	vso.normal = mul(float4(iao.normal, 0.0), invModelMat).xyz;
	vso.texCoord = iao.texCoord;

	return vso;
}