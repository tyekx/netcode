struct IAOutput {
	float3 position : POSITION;
	float3 color : COLOR;
};

struct VSOutput {
	float4 position : SV_Position;
	float3 color : COLOR;
};

cbuffer PerFrameData : register(b0) {
	float4x4 viewProj;
}

#define DEBUGPRIM_RS "RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ), RootConstants(Num32BitConstants=16, b0)"

[RootSignature(DEBUGPRIM_RS)]
VSOutput main(IAOutput iao) {
	VSOutput vso;
	vso.position = mul(float4(iao.position, 1.0f), viewProj);
	vso.color = iao.color;
	return vso;
}
