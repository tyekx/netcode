#include "RootSignatures.hlsli"
#include "Lighting.hlsli"

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

cbuffer PerFrameCb : register(b3) {
	float4x4 viewProj;
	float4 eyePos;
	Light light;
}

[RootSignature(AvatarRootSignature)]
float4 main(VSOutput vso) : SV_Target{

	float3 shade = ComputeDirectionalLight(light, material, normalize(vso.normal), normalize((eyePos - vso.position).xyz));

	return float4(shade, 1.0f);
}