#include "RootSignatures.hlsli"

struct IAOutput {
	float3 position : POSITION;
	float3 color : COLOR;
};

struct VSOutput {
	float4 position : SV_Position;
	float3 color : COLOR;
};

cbuffer PerFrameCb : register(b0) {
	float4x4 viewProj;
}

cbuffer PerObjectCb : register(b1) {
	float4x4 model;
	float4x4 invModel;
}

[RootSignature(DebugPhysxRS)]
VSOutput main( IAOutput iao ) 
{

	VSOutput vso;

	vso.color = iao.color;
	vso.position = mul(mul(float4(iao.position, 1), model), viewProj);


	return vso;
}