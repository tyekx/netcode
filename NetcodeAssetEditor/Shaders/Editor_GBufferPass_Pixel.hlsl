#include "../../NetcodeClient/Shaders/BRDF.hlsli"

struct GBufferPass_PixelInput {
	float4 position : SV_POSITION;
	float3 worldPosition : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
};

struct Light {
	float4 intensity;
	float3 position;
	float referenceDistance;
	float3 direction;
	float minDistance;
	float maxDistance;
	float angleScale;
	float angleOffset;
	uint type;
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

cbuffer LightingData : register(b3) {
	Light lights[4];
	float4 ambientLightIntensity;
	int numLights;
};

struct BrdfMaterialData {
	float3 diffuseColor;
	float reflectance;
	float3 specularColor;
	float roughness;
	float2 tiles;
	float2 tilesOffset;
	float displacementScale;
	float displacementBias;
	float textureSpaceChirality;
	uint textureFlags;
};

ConstantBuffer<BrdfMaterialData> material : register(b4);

Texture2D<float4> diffuseTexture : register(t0);
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

float3 SampleDiffuseColor(BrdfMaterialData mat, float2 texCoord) {
	float3 clr = mat.diffuseColor;

	if(IsBitSet(mat.textureFlags, DIFFUSE_TEXTURE)) {
		clr *= diffuseTexture.Sample(linearWrapSampler, texCoord);
	}

	return saturate(clr);
}

float3 SampleNormal(BrdfMaterialData mat, float2 texCoord) {
	if(IsBitSet(material.textureFlags, NORMAL_TEXTURE)) {
		return normalize(2.0f * normalTexture.Sample(linearWrapSampler, texCoord) - 1.0f);
	} else {
		return float3(0.0f, 0.0f, 1.0f);
	}
}

float SampleRoughness(BrdfMaterialData mat, float2 texCoord) {
	if(IsBitSet(mat.textureFlags, ROUGHNESS_TEXTURE)) {
		return saturate(roughnessTexture.Sample(linearWrapSampler, texCoord).x);
	} else {
		return mat.roughness;
	}
}

float3 SampleAmbientAccess(BrdfMaterialData mat, float2 texCoord) {
	float3 ambientAccess = float3(1.0f, 1.0f, 1.0f);
	if(IsBitSet(mat.textureFlags, AMBIENT_TEXTURE)) {
		ambientAccess *= ambientTexture.Sample(linearWrapSampler, texCoord);
	}
	return saturate(ambientAccess);
}

float4 main(GBufferPass_PixelInput input) : SV_TARGET
{
	float2 texCoord = input.texCoord * material.tiles;
	float3 normal = normalize(input.normal);
	float3 tangent = normalize(input.tangent - dot(input.tangent, normal) * normal);
	float3 binormal = cross(normal, tangent);
	float3x3 tbn = float3x3(tangent, -binormal, normal);

	float3 toEye = normalize(eyePos.xyz - input.worldPosition * eyePos.w);

	const float3 diffuseColor = SampleDiffuseColor(material, texCoord).xyz;
	const float roughness = SampleRoughness(material, texCoord);

	float alphaG = roughness * roughness;
	const float ambientAccess = SampleAmbientAccess(material, texCoord).x;
	float3 f0 = 0.16f * material.reflectance * material.reflectance;
	float3 V = mul(tbn, toEye);
	float3 N = SampleNormal(material, texCoord);
	float NdotV = abs(dot(N, V)) + 0.0001f;

	float3 litColor = float3(0.0f, 0.0f, 0.0f);

	for(int i = 0; i < numLights; i++) {

		Light l = lights[i];
		float3 toLightWorld = l.position;

		if(l.type != 0) {
			toLightWorld -= input.worldPosition;
		}
		
		float toLightWorldLength = length(toLightWorld);
		float3 toLight = toLightWorld / toLightWorldLength;

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
			attenuation = DistanceAttenuation(toLightWorldLength, l.referenceDistance, l.minDistance, l.maxDistance);
		}
	
		if(l.type == 2) { // SPOT LIGHT
			float cosU = dot(-toLight, l.direction);

			attenuation = DistanceAttenuation(toLightWorldLength, l.referenceDistance, l.minDistance, l.maxDistance) *
						  AngleAttenuation(cosU, l.angleScale, l.angleOffset);
		}
		
		float D = D_GGX(NdotH, alphaG);
		float G = G_SmithGGXCorrelated(NdotL, NdotV, alphaG);
		float3 F = F_SchlickFresnel(f0, 1.0f, LdotH);
		float3 Fspec = F * G * D; // / PI 
		float3 Fdiff = diffuseColor * R_DisneyDiffuse(NdotV, NdotL, LdotH, roughness); // /PI

		litColor += attenuation * NdotL * l.intensity * (Fdiff + Fspec); // * PI => PI terms cancel
	}

	return float4(litColor + diffuseColor * ambientAccess * ambientLightIntensity.xyz, 1.0f);
}
