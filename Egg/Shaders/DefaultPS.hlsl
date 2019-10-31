#include "RootSignatures.hlsli"

struct VSOutput {
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

Texture2D diffuseTex : register(t0);
SamplerState samplr;

[RootSignature(RootSig1)]
float4 main(VSOutput vso) : SV_Target{
	return float4(diffuseTex.Sample(samplr, vso.texCoord).xyz, 1.0f);
}
