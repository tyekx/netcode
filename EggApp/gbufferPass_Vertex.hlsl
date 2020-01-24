struct GbufferPass_VertexInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

struct GbufferPass_VertexOutput {
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

cbuffer FrameData : register(b2) {
	float4x4 viewProj;
	float4x4 viewProjInv;
	float4x4 view;
	float4x4 proj;
	float4x4 viewInv;
	float4x4 projInv;
	float4x4 projTex;
	float4 eyePos;
	float nearZ;
	float farZ;
	float aspectRatio;
	float fov;
};

#define GBUFFERPASS_ROOTSIGNATURE "RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ), CBV(b0), CBV(b1), CBV(b2)"

[RootSignature(GBUFFERPASS_ROOTSIGNATURE)]
GbufferPass_VertexOutput main(GbufferPass_VertexInput input)
{
	GbufferPass_VertexOutput output;

	output.position = mul(float4(input.position, 1), viewProj);
	output.normal = input.normal;
	output.texCoord = input.texCoord;

	return output;
}
