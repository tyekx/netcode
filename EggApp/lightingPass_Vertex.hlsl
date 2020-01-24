struct LightingPass_VertexInput {
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct LightingPass_VertexOutput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

#define LIGHTINGPASS_ROOTSIGNATURE "RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ), CBV(b0, Visibility = SHADER_VISIBILITY_PIXEL), DescriptorTable(SRV(t0, NumDescriptors = 3), Visibility = SHADER_VISIBILITY_PIXEL), StaticSampler(s0, Visibility = SHADER_VISIBILITY_PIXEL)"

[RootSignature(LIGHTINGPASS_ROOTSIGNATURE)]
LightingPass_VertexOutput main(LightingPass_VertexInput input)
{
	LightingPass_VertexOutput output;

	output.position = float4(input.position.xy, 0.0f, 1.0f);
	output.texCoord = input.texCoord;

	return output;
}
