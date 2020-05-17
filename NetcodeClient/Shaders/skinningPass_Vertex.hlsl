struct SkinningPass_VertexInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float3 weights : WEIGHTS;
	uint boneIds : BONEIDS;
};

struct SkinningPass_VertexOutput {
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

cbuffer MaterialData : register(b0) {
	float4 diffuseColor;
	float3 fresnelR0;
	float shininess;
};

cbuffer ObjectData : register(b1) {
	float4x4 model;
	float4x4 modelInv;
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

/*
StructuredBuffer<float4x4> bindTransforms : register(t0);
*/
cbuffer BoneData : register(b3) {
	float4x4 bindTransforms[128];
	float4x4 toRootTransforms[128];
}

#define SKINNING_PASS_ROOT_SIG "RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ), " \
"CBV(b0), " \
"CBV(b1), " \
"CBV(b2), " \
"CBV(b3)"
//"DescriptorTable(SRV(t0))"



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

	for(int i = 0; i < 4; ++i) {
		uint boneId = (input.boneIds >> (i * 8)) & 0xff;
		animPos += weights[i] * (mul(float4(input.position, 1), bindTransforms[boneId]).xyz);
		animNormal += weights[i] * (mul(input.normal, (float3x3)bindTransforms[boneId]));
	}

	SkinningPass_VertexOutput output;
	output.position = mul(mul(float4(animPos, 1), model), viewProj);
	output.normal = mul(modelInv, float4(animNormal, 0)).xyz;
	output.texCoord = input.texCoord;

	return output;
}
