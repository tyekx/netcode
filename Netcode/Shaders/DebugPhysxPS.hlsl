#include "RootSignatures.hlsli"

struct VSOutput {
	float4 position : SV_Position;
	float3 color : COLOR;
};

[RootSignature(DebugPhysxRS)]
float4 main(VSOutput vso) : SV_TARGET
{
	return float4(vso.color, 1.0f);
}