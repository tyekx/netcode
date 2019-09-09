#include "RootSignatures.hlsli"

struct VSOutput {
	float4 position : SV_Position;
	float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

Texture2D txt : register(t0);
SamplerState sampl : register(s0);

[RootSignature(RootSig2)]
float4 main(VSOutput vso) : SV_Target {
    return txt.Sample(sampl, vso.texCoord);
}
