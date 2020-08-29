struct EnvmapPass_PixelInput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
	float3 ray : TEXCOORD1;
};

TextureCube<float3> envTexture : register(t0);
SamplerState linearWrap : register(s0);

float4 main(EnvmapPass_PixelInput input) : SV_TARGET
{
	return float4(envTexture.SampleLevel(linearWrap, input.ray, 0), 1.0f);
}