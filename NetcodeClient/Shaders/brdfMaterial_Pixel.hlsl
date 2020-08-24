#include "Common.hlsli"
#include "BRDF.hlsli"

struct BrdfMaterial_PixelInput {
	float4 position : SV_POSITION;
	float3 worldPos : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
};

struct BrdfMaterialData {
	float3 diffuseColor;
	float reflectivity;
	float3 specularColor;
	float roughness;
	float2 tiles;
	float2 tilesOffset;
	float displacementScale;
	float displacementBias;
	float textureSpaceHandednessFactor;
	uint textureFlags;
};

StructuredBuffer<Light> lights : register(t7);
ConstantBuffer<BrdfMaterialData> material : register(b0);
ConstantBuffer<ObjectData> perObject : register(b1);
ConstantBuffer<FrameData> perFrame : register(b2);

Texture2D<float3> diffuseTexture : register(t0);
Texture2D<float3> normalTexture : register(t1);
Texture2D<float> ambientTexture : register(t2);
Texture2D<float> specularTexture : register(t3);
Texture2D<float> roughnessTexture : register(t4);
Texture2D<float> displacementTexture : register(t5);

TextureCube<float3> envmap : register(t6);

SamplerState linearWrapSampler : register(s0);

#define DIFFUSE_TEXTURE 0
#define NORMAL_TEXTURE 1
#define AMBIENT_TEXTURE 2
#define SPECULAR_TEXTURE 3
#define ROUGHNESS_TEXTURE 4
#define DISPLACEMENT_TEXTURE 5

static const float PI = 3.1415926535f;

bool IsBitSet(int value, int nthBit) {
	return (value & (1 << nthBit)) > 0;
}

float3 SampleDiffuseColor(uint flags, float2 texCoord, float3 diffuseColor) {
	if(IsBitSet(flags, DIFFUSE_TEXTURE)) {
		return diffuseColor * diffuseTexture.Sample(linearWrapSampler, texCoord);
	} else {
		return diffuseColor;
	}
}

float SampleRoughness(uint flags, float2 texCoord, float roughnessWithoutTexture) {
	if(IsBitSet(flags, ROUGHNESS_TEXTURE)) {
		return saturate(roughnessTexture.Sample(linearWrapSampler, texCoord).x);
	} else {
		return roughnessWithoutTexture;
	}
}

float3 SampleNormal(uint flags, float2 texCoord) {
	if(IsBitSet(flags, NORMAL_TEXTURE)) {
		return normalize(2.0f * normalTexture.Sample(linearWrapSampler, texCoord) - 1.0f);
	} else {
		return float3(0.0f, 0.0f, 1.0f);
	}
}

float3 SampleAmbientAccess(uint flags, float2 texCoord) {
	if(IsBitSet(flags, AMBIENT_TEXTURE)) {
		return ambientTexture.Sample(linearWrapSampler, texCoord).xxx;
	} else {
		return float3(1.0f, 1.0f, 1.0f);
	}
}

float4 main(BrdfMaterial_PixelInput input) : SV_Target {
	float2 texCoord = input.texCoord;
	float3 normal = normalize(input.normal);
	float3 tangent = normalize(input.tangent - dot(input.tangent, normal) * normal);
	float3 binormal = cross(normal, tangent);
	float3x3 tbn = float3x3(tangent, binormal, normal);

	float3 toEye = normalize(perFrame.eyePos.xyz - input.worldPos * perFrame.eyePos.w);

	float3 diffuseColor = SampleDiffuseColor(material.textureFlags, texCoord, material.diffuseColor);
	float3 ambientAccess = SampleAmbientAccess(material.textureFlags, texCoord);
	float roughness = SampleRoughness(material.textureFlags, texCoord, material.roughness);
	float alphaG = roughness * roughness;
	float f0 = 0.16f * material.reflectivity * material.reflectivity;
	float3 N = SampleNormal(material.textureFlags, texCoord);
	float3 V = mul(tbn, toEye);
	float NdotV = abs(dot(N, V)) + 0.0001f;

	float3 litColor = float3(0.0f, 0.0f, 0.0f);

	for(int i = 0; i < perObject.lightsCount; i++) {
		Light l = lights[perObject.lightsOffset + i];

		float3 toLightWorld = l.position;

		if(l.type > 0) {
			// homogenous w does not fit into the Light struct for now
			// if it does: toLightWorld = l.position - input.worldPos * l.position.w;
			toLightWorld -= input.worldPos;
		}

		float surfaceToLightDistance = length(toLightWorld);
		float3 toLight = toLightWorld / surfaceToLightDistance;

		if(dot(toLight, normal) < 0.0f) {
			continue;
		}

		float3 L = mul(tbn, toLight);
		float3 H = normalize(L + V);

		float LdotH = saturate(dot(L, H));
		float NdotH = saturate(dot(N, H));
		float NdotL = saturate(dot(N, L));

		float attenuation = 1.0f;

		if(l.type == 1) { // POINT LIGHT
			attenuation = DistanceAttenuation(surfaceToLightDistance, l.referenceDistance, l.minDistance, l.maxDistance);
		}

		if(l.type == 2) { // SPOT LIGHT
			attenuation *= AngleAttenuation(dot(-toLight, l.direction), l.angleScale, l.angleOffset);
		}

		float D = D_GGX(NdotH, alphaG);
		float G = G_SmithGGXCorrelated(NdotL, NdotV, alphaG);
		float3 F = F_SchlickFresnel(f0, 1.0f, LdotH);
		float3 Fspec = F * G * D / PI;
		float3 Fdiff = diffuseColor * R_DisneyDiffuse(NdotV, NdotL, LdotH, roughness);

		litColor += attenuation * NdotL * l.intensity * (Fspec + Fdiff);
	}

	return float4(litColor + diffuseColor * ambientAccess * perFrame.ambientLightIntensity.xyz, 1.0f);
}
