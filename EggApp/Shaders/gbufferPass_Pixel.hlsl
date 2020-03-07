struct GbufferPass_PixelInput {
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

struct GbufferPass_PixelOutput {
	float4 color : SV_TARGET0;
	float4 normal : SV_TARGET1;
};

cbuffer MaterialData : register(b0) {
	float4 diffuseColor;
	float3 fresnelR0;
	float shininess;
};

cbuffer FrameData : register(b2) {
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

GbufferPass_PixelOutput main(GbufferPass_PixelInput input)
{
	GbufferPass_PixelOutput output;

	output.color = diffuseColor;
	output.normal = float4(normalize(input.normal), 0.0f);

	return output;
}