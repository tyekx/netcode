struct GBufferPass_PixelInput {
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

float4 main(GBufferPass_PixelInput input) : SV_TARGET
{
	return float4(input.normal, 1.0f);
}
