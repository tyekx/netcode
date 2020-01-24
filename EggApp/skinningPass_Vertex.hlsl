struct SkinningPass_VertexInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float3 weights : WEIGHTS;
	uint boneIds : BONEIDS;
};

struct SkinningPass_VertexOutput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

cbuffer BoneData : register(b0) {
	float4x4 bindTransforms[128];
	float4x4 toRootTransforms[128];
}

#define SKINNING_PASS_ROOT_SIG "RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | ALLOW_STREAM_OUTPUT ), CBV(b0)"

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
	output.position = animPos;
	output.normal = animNormal;
	output.texCoord = input.texCoord;
	return output;
}
