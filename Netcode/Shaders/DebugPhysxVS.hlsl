#include "RootSignatures.hlsli"

struct IAOutput {
	float3 position : POSITION;
	float3 color : COLOR;
};

struct VSOutput {
	float4 position : SV_Position;
	float3 color : COLOR;
};


cbuffer PerShapeCb : register(b0) {
	float4x4 local;
	float4x4 offset;
}

cbuffer PerObjectCb : register(b1) {
	float4x4 model;
	float4x4 invModel;
}

cbuffer PerFrameCb : register(b2) {
	float4x4 viewProj;
}

[RootSignature(DebugPhysxRS)]
VSOutput main( IAOutput iao ) 
{

	VSOutput vso;

	vso.color = iao.color;
	vso.position = mul(mul(mul(mul(float4(iao.position, 1), offset), local), model), viewProj);


	return vso;
}