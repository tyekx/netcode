#include "RootSignatures.hlsli"

struct VSOutput {
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

[RootSignature(AvatarRootSignature)]
float4 main(VSOutput vso) : SV_Target{
	return vso.position * 0.0001 + float4(vso.normal + vso.texCoord.xxx * 0.00001, 1);
}