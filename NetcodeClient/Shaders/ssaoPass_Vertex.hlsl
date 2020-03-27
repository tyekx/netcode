struct SsaoPass_VertexInput {
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct SsaoPass_VertexOutput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

SsaoPass_VertexOutput main(SsaoPass_VertexInput input)
{
	SsaoPass_VertexOutput output;

	float4 posH = float4(input.position.xy, 0.0f, 1.0f);

	output.position = posH;
	output.texCoord = input.texCoord;

	return output;
}