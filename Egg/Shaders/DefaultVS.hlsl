#include "RootSignatures.hlsli"

struct IAOutput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VSOutput {
	float4 position : SV_Position;
	float3 color : COLOR;
};

[RootSignature(RootSig0)]
VSOutput main(IAOutput iao) {
	VSOutput vso;
	vso.position = float4(iao.position, 1.0f);
	vso.color = iao.normal;
	return vso;
}
