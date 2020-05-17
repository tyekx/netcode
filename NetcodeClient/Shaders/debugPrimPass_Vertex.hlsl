struct IAOutput {
	float3 position : POSITION;
	float3 color : COLOR;
};

struct VSOutput {
	float4 position : SV_Position;
	float3 color : COLOR;
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

#define DEBUGPRIMPASS_ROOTSIGNATURE "RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ), " \
"CBV(b0)"


[RootSignature(DEBUGPRIMPASS_ROOTSIGNATURE)]
VSOutput main(IAOutput iao) {
	VSOutput vso;
	vso.position = mul(float4(iao.position, 1.0f), viewProj);
	vso.color = iao.color;
	return vso;
}
