#include "RootSignatures.hlsli"

struct IAOutput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VSOutput {
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

cbuffer PerObjectCb : register(b0) {
	float4x4 model;
}

cbuffer PerFrameCb : register(b1) {
	float4x4 viewProj;
}

[RootSignature(RootSig1)]
VSOutput main(IAOutput iao) {
	VSOutput vso;
	vso.position = mul(mul(float4(iao.position, 1.0f), model), viewProj);
	vso.texCoord = iao.texCoord;
	return vso;
}
