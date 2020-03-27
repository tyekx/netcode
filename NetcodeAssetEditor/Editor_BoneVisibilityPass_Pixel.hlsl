struct GBufferPass_PixelInput {
	float4 position : SV_POSITION;
	float3 color : COLOR0;
};

float4 main(GBufferPass_PixelInput input) : SV_TARGET
{
	clip(input.color.x - 0.01f);

	return float4(input.color.x, 0.0f, 0.0f, 1.0f);
}


