#include "Common.hlsli"

struct BrdfMaterial_VertexInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
}; 

struct BrdfMaterial_VertexOutput {
	float4 position : SV_POSITION;
	float3 worldPos : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
};

ConstantBuffer<ObjectData> perObject : register(b1);
ConstantBuffer<FrameData> perFrame : register(b2);

#define BRDF_MATERIAL_ROOTSIG "RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ), " \
"CBV(b0, Visibility = SHADER_VISIBILITY_PIXEL), " \
"CBV(b1, Visibility = SHADER_VISIBILITY_ALL), " \
"CBV(b2, Visibility = SHADER_VISIBILITY_ALL), " \
"DescriptorTable(SRV(t0, NumDescriptors = 6), Visibility = SHADER_VISIBILITY_PIXEL), " \
"DescriptorTable(SRV(t6, NumDescriptors = 1), SRV(t7, NumDescriptors = 1), Visibility = SHADER_VISIBILITY_PIXEL), " \
"StaticSampler(s0, AddressU = TEXTURE_ADDRESS_WRAP, AddressV = TEXTURE_ADDRESS_WRAP, Filter = FILTER_ANISOTROPIC, mipLODBias = 0.0f, maxAnisotropy = 8, minLOD=0.0f, Visibility = SHADER_VISIBILITY_PIXEL)"


[RootSignature(BRDF_MATERIAL_ROOTSIG)]
BrdfMaterial_VertexOutput main(BrdfMaterial_VertexInput input)
{
	BrdfMaterial_VertexOutput output;

	float4 hPos = mul(float4(input.position, 1.0f), perObject.model);

	output.position = mul(hPos, perFrame.viewProj);
	output.worldPos = hPos.xyz / hPos.w;
	output.normal = normalize(mul(perObject.invModel, float4(input.normal, 0.0f)).xyz);
	output.tangent = normalize(mul(perObject.invModel, float4(input.tangent, 0.0f)).xyz);
	output.texCoord = input.texCoord;

	return output;
}
