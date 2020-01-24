const static int SAMPLE_COUNT = 14;

struct SsaoOcclusionPass_PixelInput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

cbuffer FrameData : register(b0) {
	float4x4 viewProj;
	float4x4 viewProjInv;
	float4x4 view;
	float4x4 proj;
	float4x4 viewInv;
	float4x4 projInv;
	float4x4 projTex;
	float4 eyePos;
	float nearZ;
	float farZ;
	float aspectRatio;
	float fov;
};

cbuffer SsaoData : register(b1) {
	float4 offsetVectors[SAMPLE_COUNT];
	float occlusionRadius;
	float occlusionFadeStart;
	float occlusionFadeEnd;
	float surfaceEpsilon;
}

SamplerState linearWrap : register(s0);
SamplerState pointClamp : register(s1);

Texture2D normals : register(t0);
Texture2D depth : register(t1);
Texture2D randomVectors : register(t2);

float OcclusionFunction(float distZ) {
	float occlusion = 0.0f;

	if(distZ > surfaceEpsilon) {
		float fadeLen = occlusionFadeEnd - occlusionFadeStart;

		occlusion = saturate((occlusionFadeEnd - distZ) / fadeLen);
	}

	return occlusion;
}

#define SSAO_OCCLUSION_ROOT_SIG "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), CBV(b0, Visibility = SHADER_VISIBILITY_PIXEL), CBV(b1, Visibility = SHADER_VISIBILITY_PIXEL), DescriptorTable(SRV(t0, NumDescriptors=3), Visibility = SHADER_VISIBILITY_PIXEL)," \
					"StaticSampler(s0, AddressU = TEXTURE_ADDRESS_WRAP, AddressV = TEXTURE_ADDRESS_WRAP, Filter = FILTER_MIN_MAG_MIP_LINEAR, Visibility  = SHADER_VISIBILITY_PIXEL)," \
					"StaticSampler(s1, AddressU = TEXTURE_ADDRESS_CLAMP, AddressV = TEXTURE_ADDRESS_CLAMP, Filter = FILTER_MIN_MAG_MIP_POINT, Visibility  = SHADER_VISIBILITY_PIXEL)"

[RootSignature(SSAO_OCCLUSION_ROOT_SIG)]
float main(SsaoOcclusionPass_PixelInput input) : SV_TARGET
{
	// normal in view space
	float3 n = normalize(mul(float4(normals.Sample(pointClamp, input.texCoord).xyz, 0), view).xyz);

	// depth in NDC for pixel P
	float pz = depth.Sample(pointClamp, input.texCoord).x;

	float2 ndcTex = 2.0f * float2(input.texCoord.x, input.texCoord.y) - 1.0f;

	float4 ph = mul(float4(ndcTex, pz, 1.0f), projInv);
	float3 p = ph.xyz / ph.w;

	float3 randVec = 2.0f * randomVectors.Sample(linearWrap, 4.0f * input.texCoord).xyz - 1.0f;

	float occlusionSum = 0.0f;

	for(int i = 0; i < SAMPLE_COUNT; ++i) {
		// take an offset vector
		float3 offset = reflect(normalize(offsetVectors[i].xyz), randVec);

		// check if its toward the wrong side of the view space normal
		float flip = sign(dot(offset, n));

		// flip it if so, q will be the potential occlusion point
		float3 q = p + flip * surfaceEpsilon * offset;

		float4 projQ = mul(float4(q, 1.0f), proj);
		projQ /= projQ.w;

		// sample the projected point's depth
		float rz = depth.Sample(pointClamp, 0.5f * (projQ.xy + 1.0f)).x;

		float4 rh = mul(float4(projQ.xy, rz, 1.0f), projInv);
		float3 r = rh.xyz / rh.w;

		// if distZ is positive that means r could occlude p
		float distZ = p.z - r.z;

		// check for self occlusion, if the r point is perpendicular to the surface normal, we are probably on the same triangle
		float dp = max(dot(n, normalize(r - p)), 0.0f);

		// acculumate occlusion values
		float occlusion = dp * OcclusionFunction(distZ);
		occlusionSum += occlusion;
	}

	occlusionSum /= float(SAMPLE_COUNT);

	float access = 1.0f - occlusionSum;

	return saturate(access * access);
}
