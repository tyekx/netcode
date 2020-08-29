/*
Small shader library with BRDF functions
*/
static const float NC_PI = 3.1415926535f;

// F0 approximation for F_ functions
float F0_ApproximateFromAir(float ior) {
	return (ior - 1.0f) / (ior + 1.0f);
}

// F0 approximation for F_ functions
float F0_Approximate(float n1, float n2) {
	const float m = n1 - n2;
	const float p = n1 + n2;
	return (m * m) / (p * p);
}

// fd90: upper limit to target when the incident angle is cos90, usually white (=1)
float3 F_SchlickFresnel(float3 f0, float fd90, float u) {
	return f0 + (fd90 - f0) * (pow(1.0f - u, 5.0f));
}

// Trowbridge-Reitz (GGX) microsurface normal distribution function
// alphaG: usually roughness squared, roughness: [0,1]
float D_GGX(float NdotH, float alphaG) {
	const float a2 = alphaG * alphaG;
	const float v = 1.0f + (NdotH * a2 - NdotH) * NdotH;
	return a2 / (v * v);
}

// Frostbite-like implementation of Smith geometric shadowing function
// height correlated GGX
float G_SmithGGXCorrelated(float NdotL, float NdotV, float alphaG)
{
	float alphaG2 = alphaG * alphaG;
	float NdotL2 = NdotL * NdotL;
	float NdotV2 = NdotV * NdotV;

	// OG:
	// float lambda_v = (-1.0f + sqrt(1.0f + alphaG2 * (1.0f - NdotL2) / NdotL2)) * 0.5f;
	// float lambda_l = (-1.0f + sqrt(1.0f + alphaG2 * (1.0f - NdotV2) / NdotV2)) * 0.5f;
	// return (1.0f / (1.0f + lambda_v + lambda_l)) / (4.0f * NdotL * NdotV);

	float Lambda_GGXV = NdotL * sqrt((-NdotV * alphaG2 + NdotV) * NdotV + alphaG2);
	float Lambda_GGXL = NdotV * sqrt((-NdotL * alphaG2 + NdotL) * NdotL + alphaG2);
	return 0.5f / (Lambda_GGXV + Lambda_GGXL);
}

// Energy conserving disney diffuse without modeling subsurface scattering
float R_DisneyDiffuse(float NdotV, float NdotL, float LdotH, float roughness) {
	float energyBias = lerp(0.0f, 0.5f, roughness);
	float energyFactor = lerp(1.0f, 1.0f / 1.51f, roughness);
	float fd90 = energyBias + 2.0f * LdotH * LdotH * roughness;
	float lightScatter = F_SchlickFresnel(1.0f, fd90, NdotL).x;
	float viewScatter = F_SchlickFresnel(1.0f, fd90, NdotV).x;
	return lightScatter * viewScatter * energyFactor;
}

float LightAttenuationWindow(float r, float rMax) {
	float rpm = r / rMax;
	float v = max((1.0f - rpm * rpm * rpm * rpm), 0.0f);
	return v * v;
}

float DistanceAttenuation(float r, float r0, float rMin, float rMax) {
	float rp = r0 / (max(r, rMin));
	return (rp * rp) * LightAttenuationWindow(r, rMax);
}

float AngleAttenuation(float cosU, float angleScale, float angleOffset) {
	float att = saturate(cosU * angleScale - angleOffset);
	return att * att;
}

// low discrepancy sequence with fixed sequence length
float2 Hammersley2(uint idx, uint numSamples) {
	// 1 * 2^-32
	const float twoOnThePowerOfMinus32 = asfloat(0x2F800000);

	float xCoord = float(idx) / float(numSamples);
	float yCoord = reversebits(idx) * twoOnThePowerOfMinus32;
	return float2(xCoord, yCoord);
}

// Karis importance sample GGX
float3 ImportanceSampleGGX(float2 xi, float roughness, float3 N)
{
	float a = roughness * roughness;
	float phi = 2 * NC_PI * xi.x;
	float cosTheta = sqrt((1 - xi.y) / (1 + (a * a - 1) * xi.y));
	float sinTheta = sqrt(1 - cosTheta * cosTheta);
	float3 H;
	H.x = sinTheta * cos(phi);
	H.y = sinTheta * sin(phi);
	H.z = cosTheta;
	float3 upVector = abs(N.z) < 0.999f ? float3(0, 0, 1) : float3(1, 0, 0);
	float3 tangentX = normalize(cross(upVector, N));
	float3 tangentY = cross(N, tangentX);
	return tangentX * H.x + tangentY * H.y + N * H.z;
}
