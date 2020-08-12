struct GBufferPass_PixelInput {
	float4 position : SV_POSITION;
	float3 worldPosition : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
};

struct Light {
	float4 intensity;
	float4 position;
	float falloffStart;
	float falloffEnd;
	float spotPower;
	float __lightPad0;
	float3 direction;
	float __lightPad1;
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

struct Material {
	float4 diffuseColor;
	float4 ambientColor;
	float3 fresnelR0;
	float roughness;
	float2 tiles;
	uint texturesFlags;
};

ConstantBuffer<Material> material : register(b4);

Texture2D<float4> diffuseTexture : register(t0);
Texture2D<float3> normalTexture : register(t1);
Texture2D<float> ambientTexture : register(t2);
Texture2D<float> specularTexture : register(t3);
Texture2D<float> roughnessTexture : register(t4);
Texture2D<float> heightTexture : register(t5);

TextureCube<float3> envmap : register(t6);

SamplerState linearWrapSampler : register(s0);

#define DIFFUSE_TEXTURE 0
#define NORMAL_TEXTURE 1
#define AMBIENT_TEXTURE 2
#define SPECULAR_TEXTURE 3
#define ROUGHNESS_TEXTURE 4
#define HEIGHT_TEXTURE 5

bool IsBitSet(int value, int nthBit) {
	return (value & (1 << nthBit)) > 0;
}

float3 Fresnel(Material mat, float3 toLight, float3 halfway) {
	float ndoth = saturate(dot(halfway, toLight));
	float f0 = 1.0f - ndoth;
	float3 R0 = mat.fresnelR0;
	return R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);
}

float4 SampleDiffuseColor(Material mat, float2 texCoord) {
	float4 clr = mat.diffuseColor;

	if(IsBitSet(mat.texturesFlags, DIFFUSE_TEXTURE)) {
		clr *= diffuseTexture.Sample(linearWrapSampler, texCoord);
	}

	return saturate(clr);
}

float SampleRoughness(Material mat, float2 texCoord) {
	float roughness = mat.roughness;

	if(IsBitSet(mat.texturesFlags, ROUGHNESS_TEXTURE)) {
		roughness *= (1.0f - roughnessTexture.Sample(linearWrapSampler, texCoord).x);
	}

	return saturate(roughness);
}

float3 SampleAmbientAccess(Material mat, float2 texCoord) {
	float3 ambientAccess = float3(1.0f, 1.0f, 1.0f);
	if(IsBitSet(mat.texturesFlags, AMBIENT_TEXTURE)) {
		ambientAccess *= ambientTexture.Sample(linearWrapSampler, texCoord);
	}
	return saturate(ambientAccess);
}

float3 BlinnPhong(Material mat, float2 texCoord, float3 lightIntensity, float3 toLight, float3 normal, float3 toEye)
{
	float roughnessSample = SampleRoughness(mat, texCoord);

	const float m = (1.0f - roughnessSample) * 256.0f;
	float3 halfway = normalize(toEye + toLight);

	float roughnessFactor = (m + 8.0f) * pow(max(dot(halfway, normal), 0.0f), m) / 8.0f;
	float3 fresnelFactor = Fresnel(mat, toLight, halfway);

	float3 specAlbedo = fresnelFactor * roughnessFactor;

	specAlbedo = specAlbedo / (specAlbedo + 1.0f);

	float3 diffuseSample = SampleDiffuseColor(mat, texCoord).xyz;
	float3 ambientAccess = SampleAmbientAccess(mat, texCoord).xyz;

	return (diffuseSample + specAlbedo) * lightIntensity + ambientLightIntensity.xyz * diffuseSample * ambientAccess;
}

float3 ComputeDirectionalLight(Material mat, float2 texCoord, float3 lightIntensity, float3 toLight, float3 normal, float3 toEye)
{
	float ndotl = max(dot(toLight, normal), 0.1f);

	return BlinnPhong(mat, texCoord, lightIntensity * ndotl, toLight, normal, toEye);
}

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f * normalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N) * N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

float4 main(GBufferPass_PixelInput input) : SV_TARGET
{
	float2 texCoord = input.texCoord;
	float3 normal = normalize(input.normal);

	Light l = lights[0];

	float3 toLight = normalize((l.position.xyz - input.worldPosition * l.position.w));
	float3 toEye = normalize(eyePos.xyz - input.worldPosition);

	if(IsBitSet(material.texturesFlags, NORMAL_TEXTURE)) {
		float3 tangent = normalize(input.tangent - dot(input.tangent, normal) * normal);
		float3 binormal = cross(normal, tangent);
		float3x3 tbn = float3x3(tangent, binormal, normal);

		float3 normalSample = 2.0f * normalTexture.Sample(linearWrapSampler, texCoord) - 1.0f;

		normal = mul(normalSample, tbn);
	}

	float metallicWeight = 0.0f;
	float3 envMapSample = float3(1.0f, 1.0f, 1.0f);
	if(IsBitSet(material.texturesFlags, SPECULAR_TEXTURE)) {
		metallicWeight = specularTexture.Sample(linearWrapSampler, texCoord).x;

		float3 reflectedDir = reflect(-toEye, normal);
		envMapSample = envmap.Sample(linearWrapSampler, reflectedDir);
	}

	float3 shaded =
		(1.0f - metallicWeight) * ComputeDirectionalLight(material, texCoord, l.intensity.xyz, toLight, normal, toEye) +
				metallicWeight  * envMapSample;

	return float4(shaded, 1.0f);
}
