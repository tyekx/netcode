struct EnvmapPass_VertexInput {
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct EnvmapPass_VertexOutput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
	float3 ray : TEXCOORD1;
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

#define ENVMAP_ROOT_SIG "RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_HULL_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS | DENY_DOMAIN_SHADER_ROOT_ACCESS )," \
						"CBV(b0, Visibility = SHADER_VISIBILITY_VERTEX), " \
						"DescriptorTable(SRV(t0, NumDescriptors = 1), Visibility = SHADER_VISIBILITY_PIXEL), " \
						"StaticSampler(s0, AddressU = TEXTURE_ADDRESS_WRAP, AddressV = TEXTURE_ADDRESS_WRAP, AddressW = TEXTURE_ADDRESS_WRAP, Filter = FILTER_MIN_MAG_MIP_LINEAR, Visibility  = SHADER_VISIBILITY_PIXEL)"

[RootSignature(ENVMAP_ROOT_SIG)]
EnvmapPass_VertexOutput main( EnvmapPass_VertexInput input )
{
	EnvmapPass_VertexOutput output;
	output.position = float4(input.position.xy, 0.99999f, 1.0f);
	float4 hWorldPosMinusEye = mul(float4(input.position, 1.0f), rayDir);
	hWorldPosMinusEye /= hWorldPosMinusEye.w;
	output.ray = hWorldPosMinusEye.xyz;
	output.texCoord = input.texCoord;
	return output;
}
