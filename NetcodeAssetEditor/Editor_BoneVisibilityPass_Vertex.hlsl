struct GBufferPass_VertexInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float3 weights : WEIGHTS;
	uint boneIds : BONEIDS;
};

struct GBufferPass_VertexOutput {
	float4 position : SV_POSITION;
	float3 color : COLOR0;
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
}

cbuffer ObjectData : register(b2) {
	float4x4 model;
	float4x4 modelInv;
}

cbuffer BoneVisibilityData : register(b3) {
	uint4 boneVisibility[32];
}

#define GBUFFER_PASS_ROOT_SIG "RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ), CBV(b0), CBV(b1), CBV(b2), CBV(b3)"

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

	float color = 0.0f;

	for(int i = 0; i < 4; ++i) {
		uint boneId = (input.boneIds >> (i * 8)) & 0xff;
		animPos += weights[i] * (mul(float4(input.position, 1), bindTransforms[boneId]).xyz);
		animNormal += weights[i] * (mul(input.normal, (float3x3)bindTransforms[boneId]));

		uint v = boneVisibility[boneId / 4][boneId % 4];

		if(boneId != 0xff && v == 1) {
			color += weights[i];
		}
	}

	GBufferPass_VertexOutput output;

	float4 hPos = mul(mul(float4(animPos + 1.0f * animNormal, 1.0f), model), viewProj);
	//float3 normal = mul(modelInv, animNormal);
	output.position = hPos;
	output.color = float3(color, color, color);

	return output;
}
