struct GBufferPass_VertexInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float3 weights : WEIGHTS;
	uint boneIds : BONEIDS;
};

struct GBufferPass_VertexOutput {
	float4 position : SV_POSITION;
	float3 worldPosition : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
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

cbuffer BoneData : register(b1) {
	float4x4 bindTransforms[128];
	float4x4 toRootTransforms[128];
};

cbuffer ObjectData : register(b2) {
	float4x4 model;
	float4x4 modelInv;
};

cbuffer MaterialData : register(b3) {
	float4 diffuseColor;
	float4 ambientColor;
	float3 fresnelR0;
	float roughness;
	float2 tiles;
	int texturesFlags;
};

#define GBUFFER_PASS_ROOT_SIG "RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ), " \
"CBV(b0, Visibility = SHADER_VISIBILITY_ALL), " \
"CBV(b1, Visibility = SHADER_VISIBILITY_VERTEX), " \
"CBV(b2, Visibility = SHADER_VISIBILITY_VERTEX), " \
"CBV(b3, Visibility = SHADER_VISIBILITY_PIXEL), " \
"CBV(b4, Visibility = SHADER_VISIBILITY_PIXEL), " \
"DescriptorTable(SRV(t0, NumDescriptors = 6), Visibility = SHADER_VISIBILITY_PIXEL), " \
"DescriptorTable(SRV(t6, NumDescriptors = 1), Visibility = SHADER_VISIBILITY_PIXEL), " \
"StaticSampler(s0, AddressU = TEXTURE_ADDRESS_WRAP, AddressV = TEXTURE_ADDRESS_WRAP, Filter = FILTER_MIN_MAG_MIP_LINEAR, Visibility = SHADER_VISIBILITY_PIXEL)"

[RootSignature(GBUFFER_PASS_ROOT_SIG)]
GBufferPass_VertexOutput main(GBufferPass_VertexInput input)
{
	float weights[4] = {
		input.weights[0],
		input.weights[1],
		input.weights[2],
		1.0f - input.weights[0] - input.weights[1] - input.weights[2]
	};

	float3 animPos = float3(0, 0, 0);
	float3 animNormal = float3(0, 0, 0);
	float3 animTangent = float3(0, 0, 0);

	[unroll]
	for(int i = 0; i < 4; ++i) {
		uint boneId = (input.boneIds >> (i * 8)) & 0xff;
		animPos += weights[i] * (mul(float4(input.position, 1), bindTransforms[boneId]).xyz);
		animNormal += weights[i] * (mul(input.normal, (float3x3)bindTransforms[boneId]));
		animTangent += weights[i] * (mul(input.tangent, (float3x3)bindTransforms[boneId]));
	}

	float4 hPos = mul(float4(animPos, 1.0f), model);
	hPos.xyz /= hPos.w;

	GBufferPass_VertexOutput output;
	output.position = mul(hPos, viewProj);
	output.worldPosition = hPos.xyz;
	output.normal = mul(modelInv, float4(animNormal, 0.0f)).xyz;
	output.tangent = mul(modelInv, float4(animTangent, 0.0f)).xyz;
	output.texCoord = input.texCoord;
	return output;
}
