#include "RootSignatures.hlsli"

struct IAOutput {
	float3 position : POSITION;
	float3 color : COLOR;
};

struct VSOutput {
	float4 position : SV_Position;
	float3 color : COLOR;
};


cbuffer PerRayCb : register(b0) {
	float3 dir;
	float length;
	float3 startPos;
	float timeLeft;
	float3 color;
}

cbuffer PerFrameCb : register(b1) {
	float4x4 viewProj;
}

[RootSignature(DebugPhysxRayRS)]
VSOutput main(IAOutput iao)
{

	VSOutput vso;

	float3 pos = startPos + dir * iao.position.z * length;

	vso.color = color * saturate(timeLeft);
	vso.position = mul(float4(pos, 1), viewProj);

	return vso;
}
