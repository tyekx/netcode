#include "BRDF.hlsli"

#define ROOTSIG "DescriptorTable(UAV(u0, NumDescriptors = 1)), " \
"DescriptorTable(SRV(t0, NumDescriptors = 1)), " \
"RootConstants(Num32BitConstants = 2, b0), " \
"StaticSampler(s0, Filter = FILTER_MIN_MAG_MIP_LINEAR)"

cbuffer PassData : register(b0) {
	uint numMipLevels;
	uint currentMipLevelIndex;
}

RWTexture2DArray<float4> destTextureCube : register(u0);
TextureCube<float4> sourceEnvMap : register(t0);

SamplerState sourceEnvMapSampler : register(s0);

float3 PrefilterEnvMap(float Roughness, float3 R)
{
	float3 N = R;
	float3 V = R;
	float3 prefilteredColor = 0;
	float totalWeight = 0;
	const uint NUM_SAMPLES = 1024;
	for(uint i = 0; i < NUM_SAMPLES; i++)
	{
		float2 Xi = Hammersley2(i, NUM_SAMPLES);
		float3 H = ImportanceSampleGGX(Xi, Roughness, N);
		float3 L = 2.0f * dot(V, H) * H - V;
		float NdotL = saturate(dot(N, L));
		if(NdotL > 0)
		{
			prefilteredColor += sourceEnvMap.SampleLevel(sourceEnvMapSampler, L, 0).rgb * NdotL;
			totalWeight += NdotL;
		}
	}

	return prefilteredColor / totalWeight;
}

[RootSignature(ROOTSIG)]
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint height;
	uint width;
	uint elements;
	destTextureCube.GetDimensions(height, width, elements);

	const float3 lightDir = normalize(float3(0.2f, 1.0f, -1.0f));

	const float w = float(width);
	const float wPer2 = float(width / 2);
	const float u = float(DTid.x);
	const float v = float(DTid.y);
	const float3 center = wPer2;

	const float3 xDir = normalize(float3(wPer2, (w - 1) - v, u) - float3(0, center.yz));
	const float3 xNegDir = normalize(float3(-wPer2, (w - 1) - v, (w - 1) - u) - float3(0, center.yz));
	const float3 yDir = normalize(float3((w - 1) - u, wPer2, v) - float3(center.x, 0, center.z));
	const float3 yNegDir = normalize(float3((w - 1) - u, -wPer2, (w - 1) - v) - float3(center.x, 0, center.z));
	const float3 zDir = normalize(float3((w - 1) - u, (w - 1) - v, wPer2) - float3(center.x, center.y, 0));
	const float3 zNegDir = normalize(float3(u, (w - 1) - v, -wPer2) - float3(center.x, center.y, 0));

	const float roughness = float(currentMipLevelIndex) / float(numMipLevels - 1);

	const uint negX = 0;
	const uint plusX = 1;
	const uint plusY = 2;
	const uint negY = 3;
	const uint negZ = 5;
	const uint plusZ = 4;

	destTextureCube[uint3(DTid.xy, plusY)] = float4(PrefilterEnvMap(roughness, yDir), 1.0f);
	destTextureCube[uint3(DTid.xy, negY)] = float4(PrefilterEnvMap(roughness, yNegDir), 1.0f);
	destTextureCube[uint3(DTid.xy, plusX)] = float4(PrefilterEnvMap(roughness, xDir), 1.0f);
	destTextureCube[uint3(DTid.xy, negX)] = float4(PrefilterEnvMap(roughness, xNegDir), 1.0f);
	destTextureCube[uint3(DTid.xy, plusZ)] = float4(PrefilterEnvMap(roughness, zDir), 1.0f);
	destTextureCube[uint3(DTid.xy, negZ)] = float4(PrefilterEnvMap(roughness, zNegDir), 1.0f);
}
