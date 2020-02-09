struct ColliderPass_VertexInput {
	float3 position : POSITION;
};

struct ColliderPass_VertexOutput {
	float4 position : SV_POSITION;
};

cbuffer ColliderData : register(b0) {
	float4x4 localTransform;
	float4 color;
	uint boneReference;
}

cbuffer FrameData : register(b1) {
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

cbuffer BoneData : register(b2) {
	float4x4 bindTransforms[128];
	float4x4 toRootTransforms[128];
}

cbuffer ObjectData : register(b3) {
	float4x4 model;
	float4x4 modelInv;
}

#define COLLIDER_PASS_ROOT_SIGNATURE "RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ), CBV(b0), CBV(b1), CBV(b2), CBV(b3)"

[RootSignature(COLLIDER_PASS_ROOT_SIGNATURE)]
ColliderPass_VertexOutput main(ColliderPass_VertexInput input) {

	ColliderPass_VertexOutput output;

	if(boneReference < 0x7F) {
		output.position =
			mul(
				mul(
					mul(
						mul(
							float4(input.position, 1.0f),
							localTransform
						),
						toRootTransforms[boneReference]
					),
					model
				),
				viewProj
			);

	} else {
		output.position =
			mul(
				mul(
					mul(
						float4(input.position, 1.0f),
						localTransform
					),
					model
				),
				viewProj
			);
	}

	return output;
}

