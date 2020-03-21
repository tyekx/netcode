const static int SAMPLE_COUNT = 14;
const static int BLUR_RADIUS = 5;

struct SsaoPass_PixelInput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

Texture2D<float> ssaoOcclusion : register(t0);
Texture2D<float3> normals : register(t1);
Texture2D<float> depth : register(t2);

SamplerState linearWrap : register(s0);
SamplerState pointClamp : register(s1);

cbuffer SsaoData : register(b0) {
	float4 offsetVectors[SAMPLE_COUNT];
	float occlusionRadius;
	float occlusionFadeStart;
	float occlusionFadeEnd;
	float surfaceEpsilon;
	float4 blurWeights[3];
	float2 invRenderTargetSize;
}

cbuffer SsaoBlurData : register(b1) {
	uint horizontalBlur;
}

cbuffer FrameData : register(b2) {
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

#define SSAO_BLUR_PASS_ROOTSIG "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), DescriptorTable(SRV(t0, NumDescriptors=3), Visibility = SHADER_VISIBILITY_PIXEL), CBV(b0), CBV(b2), " \
							   "RootConstants(num32BitConstants=1, b1), " \
							   "StaticSampler(s0, AddressU = TEXTURE_ADDRESS_WRAP, AddressV = TEXTURE_ADDRESS_WRAP, Filter = FILTER_MIN_MAG_MIP_LINEAR, Visibility  = SHADER_VISIBILITY_PIXEL)," \
							   "StaticSampler(s1, AddressU = TEXTURE_ADDRESS_CLAMP, AddressV = TEXTURE_ADDRESS_CLAMP, Filter = FILTER_MIN_MAG_MIP_POINT, Visibility  = SHADER_VISIBILITY_PIXEL)"
/*
 main(SsaoPass_PixelInput input) : SV_TARGET
{
	return ssaoOcclusion.Sample(linearClamp, input.texCoord);
}*/

float3 NDCToViewSpace(float2 tex, float z) {
	float4 hPos = mul(float4(2.0f * tex - 1.0f, z, 1.0f), projInv);
	return hPos.xyz / hPos.w;
}

[RootSignature(SSAO_BLUR_PASS_ROOTSIG)]
float main(SsaoPass_PixelInput input) : SV_TARGET
{
	float weights[12] =
	{
		blurWeights[0].x, blurWeights[0].y, blurWeights[0].z, blurWeights[0].w,
		blurWeights[1].x, blurWeights[1].y, blurWeights[1].z, blurWeights[1].w,
		blurWeights[2].x, blurWeights[2].y, blurWeights[2].z, blurWeights[2].w,
	};

	float2 texOffset;
	if(horizontalBlur > 0)
	{
		texOffset = float2(invRenderTargetSize.x, 0.0f);
	} else
	{
		texOffset = float2(0.0f, invRenderTargetSize.y);
	}

	// The center value always contributes to the sum.
	float color = weights[BLUR_RADIUS] * ssaoOcclusion.Sample(pointClamp, input.texCoord);
	float totalWeight = weights[BLUR_RADIUS];

	float3 centerNormal = normals.Sample(pointClamp, input.texCoord).xyz;
	float  centerDepth = NDCToViewSpace(input.texCoord, depth.Sample(pointClamp, input.texCoord).r).z;

	for(float i = -BLUR_RADIUS; i <= BLUR_RADIUS; ++i)
	{
		// We already added in the center weight.
		if(i == 0)
			continue;

		float2 tex = input.texCoord + i * texOffset;

		float3 neighborNormal = normals.Sample(pointClamp, tex).xyz;
		float  neighborDepth = NDCToViewSpace(tex, depth.Sample(pointClamp, tex, 0.0f).r).z;

		//
		// If the center value and neighbor values differ too much (either in 
		// normal or depth), then we assume we are sampling across a discontinuity.
		// We discard such samples from the blur.
		//

		if(dot(neighborNormal, centerNormal) >= 0.6f &&
			abs(neighborDepth - centerDepth) <= 5.0f)
		{
			float weight = weights[i + BLUR_RADIUS];

			// Add neighbor pixel to blur.
			color += weight * ssaoOcclusion.SampleLevel(pointClamp, tex, 0.0);

			totalWeight += weight;
		}
	}

	// Compensate for discarded samples by making total weights sum to 1.
	return color / totalWeight;
}
