#include "Common.hlsli"

struct SkinningPass_VertexInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float3 tangent : TANGENT;
	float3 weights : WEIGHTS;
	uint boneIds : BONEIDS;
};

struct SkinningPass_VertexOutput {
	float4 position : SV_POSITION;
	float3 worldPos : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
};

ConstantBuffer<ObjectData> perObject : register(b1);
ConstantBuffer<FrameData> perFrame : register(b2);
ConstantBuffer<BoneData> boneData : register(b3);

/*
StructuredBuffer<float4x4> bindTransforms : register(t0);
*/

#define SKINNING_PASS_ROOT_SIG "RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ), " \
"CBV(b0, Visibility = SHADER_VISIBILITY_PIXEL), " \
"CBV(b1, Visibility = SHADER_VISIBILITY_ALL), " \
"CBV(b2, Visibility = SHADER_VISIBILITY_ALL), " \
"DescriptorTable(SRV(t0, NumDescriptors = 6), Visibility = SHADER_VISIBILITY_PIXEL), " \
"SRV(t6, Visibility = SHADER_VISIBILITY_PIXEL), " \
"DescriptorTable(SRV(t7, NumDescriptors = 2), Visibility = SHADER_VISIBILITY_PIXEL), " \
"CBV(b3, Visibility = SHADER_VISIBILITY_VERTEX), " \
"StaticSampler(s0, AddressU = TEXTURE_ADDRESS_WRAP, AddressV = TEXTURE_ADDRESS_WRAP, Filter = FILTER_ANISOTROPIC, mipLODBias = 0.0f, maxAnisotropy = 8, minLOD=0.0f, Visibility = SHADER_VISIBILITY_PIXEL), " \
"StaticSampler(s1, AddressU = TEXTURE_ADDRESS_CLAMP, AddressV = TEXTURE_ADDRESS_CLAMP, Filter = FILTER_MIN_MAG_MIP_LINEAR, Visibility = SHADER_VISIBILITY_PIXEL)"


[RootSignature(SKINNING_PASS_ROOT_SIG)]
SkinningPass_VertexOutput main(SkinningPass_VertexInput input)
{
	float weights[4] = {
		input.weights[0],
		input.weights[1],
		input.weights[2],
		1.0f - input.weights[0] - input.weights[1] - input.weights[2]
	};

	float3 animPos = float3(0, 0, 0);
	float3 animNormal = float3(0, 0, 0);
	float3 animTang = float3(0, 0, 0);

	for(int i = 0; i < 4; ++i) {
		uint boneId = (input.boneIds >> (i * 8)) & 0xff;
		animPos += weights[i] * (mul(float4(input.position, 1), boneData.bindTransforms[boneId]).xyz);
		animNormal += weights[i] * (mul(input.normal, (float3x3)boneData.bindTransforms[boneId]));
		animTang += weights[i] * (mul(input.tangent, (float3x3)boneData.bindTransforms[boneId]));
	}

	float4 hPos = mul(float4(animPos, 1), perObject.model);

	SkinningPass_VertexOutput output;
	output.position = mul(hPos, perFrame.viewProj);
	output.worldPos = hPos.xyz / hPos.w;
	output.normal = mul(perObject.invModel, float4(animNormal, 0)).xyz;
	output.tangent = mul(perObject.invModel, float4(animTang, 0)).xyz;
	output.texCoord = input.texCoord;
	
	return output;
}
