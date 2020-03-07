struct SsaoPass_PixelInput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

Texture2D<float> ssaoOcclusion : register(t0);
Texture2D<float> depth : register(t1);

SamplerState linearClamp : register(s0);

#define SSAO_BLUR_PASS_ROOTSIG "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), DescriptorTable(SRV(t0, NumDescriptors=2), Visibility = SHADER_VISIBILITY_PIXEL), CBV(b0)," \
							   "StaticSampler(s0, AddressU = TEXTURE_ADDRESS_CLAMP, AddressV = TEXTURE_ADDRESS_CLAMP, Filter = FILTER_MIN_MAG_MIP_LINEAR, Visibility  = SHADER_VISIBILITY_PIXEL)"

[RootSignature(SSAO_BLUR_PASS_ROOTSIG)]
float main(SsaoPass_PixelInput input) : SV_TARGET
{
	return ssaoOcclusion.Sample(linearClamp, input.texCoord);
}