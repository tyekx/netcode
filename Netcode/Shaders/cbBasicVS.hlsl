#include "RootSignatures.hlsli"

struct IAOutput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

struct VSOutput {
    float4 position : SV_Position;
	float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

cbuffer PerObjectCb : register(b0) {
	row_major float4x4 modelMat;
	row_major float4x4 invModelMat;
}

cbuffer PerFrameCb : register(b1) {
	row_major float4x4 viewMat;
	row_major float4x4 projMat;
}

[RootSignature(RootSig2)]
VSOutput main(IAOutput iao) {
	VSOutput vso;

	float4x4 mvp = mul(modelMat, mul(viewMat, projMat));
	vso.position = mul(float4(iao.position, 1.0f), mvp);
	vso.normal = mul(invModelMat, float4(iao.normal, 0.0)).xyz;
    vso.texCoord = iao.texCoord;
	return vso;
}
