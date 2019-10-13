#include "RootSignatures.hlsli"

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


cbuffer PerFrameCb : register(b0) {
	float4x4 viewMat;
	float4x4 projMat;
}

cbuffer PerObjectCb : register(b1) {
	float4x4 modelMat;
	float4x4 invModelMat;
}

cbuffer BoneDataCb : register(b2) {
	float4x4 boneTransforms[128];
}

[RootSignature(AvatarRootSignature)]
VSOutput main(IAOutput iao)
{
	float weights[4] = { iao.weights[0], iao.weights[1], iao.weights[2], 1.0f - iao.weights[0] - iao.weights[1] - iao.weights[2] };

	float3 posL = float3(0, 0, 0);
	float3 normalL = float3(0, 0, 0);

	for(int i = 0; i < 4; ++i) {
		if(iao.boneIds[i] == -1) {
			break;
		}
		posL += weights[i] * (mul(boneTransforms[iao.boneIds[i]], float4(iao.position, 1)).xyz);
		normalL += weights[i] * (mul(boneTransforms[iao.boneIds[i]], float4(iao.normal, 0)).xyz);
	}

	iao.position = posL;
	iao.normal = normalL;

	VSOutput vso;
	vso.position = mul(projMat, mul(viewMat, mul(modelMat, float4(iao.position, 1.0f))));
	vso.normal = mul(float4(iao.normal, 0.0), invModelMat).xyz;
	vso.texCoord = iao.texCoord + (boneTransforms[0][0]*0.00001);

	return vso;
}