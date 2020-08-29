#include "BRDF.hlsli"

#define ROOTSIG "DescriptorTable(UAV(u0, NumDescriptors = 1))"

RWTexture2D<float4> destLut : register(u0);

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float a = roughness;
	float k = (a * a) / 2.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = saturate(dot(N, V));
	float NdotL = saturate(dot(N, L));
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float3 IntegrateBRDF(float roughness, float NdotV)
{
	float3 V;
	V.x = sqrt(1.0f - NdotV * NdotV);
	V.y = 0;
	V.z = NdotV;
	float A = 0;
	float B = 0;
	float C = 0;
	const uint NUM_SAMPLES = 1024;

	const float3 N = float3(0, 0, 1);
	float alphaG = roughness * roughness;

	for(uint i = 0; i < NUM_SAMPLES; i++)
	{
		float2 Xi = Hammersley2(i, NUM_SAMPLES);
		float3 H = ImportanceSampleGGX(Xi, roughness, N);
		float3 L = 2 * dot(V, H) * H - V;
		float NdotL = saturate(L.z);
		float NdotH = saturate(H.z);
		float VdotH = saturate(dot(V, H));
		float LdotH = saturate(dot(L, H));

		if(NdotL > 0)
		{
			// this includes the 4 * NdotL * NdotV divisor!
			float G_Vis = G_SmithGGXCorrelated(NdotL, NdotV, alphaG);
			float Fc = pow(1.0f - VdotH, 5.0f);
			A += (1.0f - Fc) * G_Vis;
			B += Fc * G_Vis;
		}

		Xi = frac(Xi + 0.5f);

		if(NdotL > 0) {
			C += R_DisneyDiffuse(NdotV, NdotL, LdotH, roughness);
		}
	}
	return float3(A, B, C) / float3(NUM_SAMPLES, NUM_SAMPLES, NUM_SAMPLES);
}


[RootSignature(ROOTSIG)]
[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	uint lutWidth;
	uint lutHeight;

	destLut.GetDimensions(lutWidth, lutHeight);

	float roughnessValue = (DTid.y) / float(lutWidth);
	float cosTheta = (DTid.x + 1) / float(lutHeight);

	destLut[DTid.xy] = float4(IntegrateBRDF(roughnessValue, cosTheta), 0.0f);
}

