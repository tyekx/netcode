const static int SAMPLE_COUNT = 14;

struct SsaoOcclusionPass_PixelInput {
	float4 position : SV_POSITION;
	float3 positionV : POSITION;
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
	float4x4 rayDir;
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

#define SSAO_OCCLUSION_ROOT_SIG "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), CBV(b0), CBV(b1, Visibility = SHADER_VISIBILITY_PIXEL), DescriptorTable(SRV(t0, NumDescriptors=3), Visibility = SHADER_VISIBILITY_PIXEL)," \
					"StaticSampler(s0, AddressU = TEXTURE_ADDRESS_WRAP, AddressV = TEXTURE_ADDRESS_WRAP, Filter = FILTER_MIN_MAG_MIP_LINEAR, Visibility  = SHADER_VISIBILITY_PIXEL)," \
					"StaticSampler(s1, AddressU = TEXTURE_ADDRESS_CLAMP, AddressV = TEXTURE_ADDRESS_CLAMP, Filter = FILTER_MIN_MAG_MIP_POINT, Visibility  = SHADER_VISIBILITY_PIXEL)"

float GetDepthSample(float2 texCoord) {
	return depth.Sample(pointClamp, texCoord).x;
}

float3 NDCToViewSpace(float3 position) {
	float4 hPos = mul(float4(position, 1.0f), projInv);
	return hPos.xyz / hPos.w;
}

[RootSignature(SSAO_OCCLUSION_ROOT_SIG)]
float4 main(SsaoOcclusionPass_PixelInput input) : SV_TARGET
{
	float3 n = normalize(mul(normals.Sample(linearWrap, input.texCoord), view));
	float pz = GetDepthSample(input.texCoord);
	
	float3 posV = NDCToViewSpace(float3(2.0f * input.texCoord - 1.0f, pz));

	//return float4(posV.x - 100.0f, posV.y - 100.0f, posV.z, 1.0f);

	float3 randomVec = normalize(2.0f * randomVectors.Sample(linearWrap, 4.0f * input.texCoord).xyz - 1.0f);


	float3 tangent = normalize(randomVec - n * dot(randomVec, n));
	float3 binormal = normalize(cross(n, tangent));
	float3x3 tbn = float3x3(tangent, binormal, n);

	float occlusionSum = 0.0f;

	for(int i = 0; i < SAMPLE_COUNT; ++i) {

		float3 offset = mul(offsetVectors[i], transpose(tbn));

		float flip = sign(dot(offset, n));

		float3 qV = posV + flip * offset * occlusionRadius;

		float4 qH = mul(float4(qV, 1.0f), proj);
		qH.xyzw /= qH.w;

		float2 qTex = 0.5f * qH.xy + 0.5f;

		float rz = GetDepthSample(qTex);

		float3 rV = NDCToViewSpace(float3(qH.xy, rz));
		
		float distZ = rV.z - posV.z;

		float3 rNorm = normalize(mul(normals.Sample(linearWrap, qTex).xyz, view));

		float dp = max(dot(n, rNorm), 0.0f);

		occlusionSum += dp * OcclusionFunction(distZ);
	}
	
	occlusionSum /= float(SAMPLE_COUNT);

	float access = 1.0f - occlusionSum;
	access *= access;

	return float4(access, access, access, 1.0f);
}
