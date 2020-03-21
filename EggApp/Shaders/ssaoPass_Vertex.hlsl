struct SsaoPass_VertexInput {
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct SsaoPass_VertexOutput {
	float4 position : SV_POSITION;
	float3 positionV : POSITION;
	float2 texCoord : TEXCOORD;
};

cbuffer FrameData : register(b0) {
	float4x4 viewProj;
	float4x4 viewProjInv;
	float4x4 view;
	float4x4 proj;
	float4x4 viewInv;
	float4x4 projInv;
	float4x4 projTex;
	float4x4 rayDir;
	float4 eyePos;
	float nearZ;
	float farZ;
	float aspectRatio;
	float fov;
};

SsaoPass_VertexOutput main(SsaoPass_VertexInput input)
{
	SsaoPass_VertexOutput output;

	float4 posH = float4(input.position.xy, 0.0f, 1.0f);
	float4 posV = mul(posH, projInv);

	output.position = posH;
	output.positionV = posV.xyz / posV.w;
	output.texCoord = input.texCoord;

	return output;
}