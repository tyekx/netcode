struct LightingPass_PixelInput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

struct Material {
	float4 diffuseAlbedo;
	float3 fresnelR0;
	float shininess;
};

struct Light {
	float4 intensity;
	float4 position;
	float falloffStart;
	float falloffEnd;
	float spotPower;
	float __lightPad0;
	float3 direction;
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

Texture2D<float4> colors : register(t0);
Texture2D<float3> normals : register(t1);
Texture2D<float> depthBuffer : register(t2);
Texture2D<float4> ssaoBuffer : register(t3);

SamplerState linearWrapSampler : register(s0);

/*
float Attenuate(float distanceToSource, Light l) {
	return saturate((l.falloffEnd - distanceToSource) / (l.falloffEnd - l.falloffStart));
}*/

float3 Fresnel(Material mat, float ndotl) {
	float f0 = 1.0f - ndotl;
	float3 R0 = mat.fresnelR0;
	return R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);
}

float3 BlinnPhong(float3 lightIntensity, float3 lightVec, float3 normalVec, float3 toEye, Material mat)
{
	const float m = mat.shininess * 256.0f;
	float3 halfVec = normalize(toEye + lightVec);
	float ndotl = dot(normalVec, lightVec);

	float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normalVec), 0.0f), m) / 8.0f;
	float3 fresnelFactor = Fresnel(mat, ndotl);

	float3 specAlbedo = fresnelFactor * roughnessFactor;

	specAlbedo = specAlbedo / (specAlbedo + 1.0f);

	return (mat.diffuseAlbedo + specAlbedo) * lightIntensity;
}

float3 ComputeDirectionalLight(Material mat, float3 lightIntensity, float3 toLight, float3 normal, float3 toEye)
{
	float ndotl = max(dot(toLight, normal), 0.0f);

	return BlinnPhong(lightIntensity * ndotl, toLight, normal, toEye, mat);
}
/*
float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye, float2 tex)
{
	float3 lightVec = L.position.xyz - pos;

	float d = length(lightVec);

	if(d > L.falloffEnd)
		return 0.0f;

	lightVec /= d;

	float ndotl = max(dot(lightVec, normal), 0.0f);
	float3 lightIntensity = L.intensity.xyz * ndotl;

	float att = Attenuate(d, L);
	lightIntensity *= att;

	return BlinnPhong(lightIntensity, lightVec, normal, toEye, mat);
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye, float2 tex)
{
	float3 lightVec = L.position.xyz - pos;

	float d = length(lightVec);

	if(d > L.falloffEnd)
		return 0.0f;

	lightVec /= d;

	float ndotl = max(dot(lightVec, normal), 0.0f);
	float3 lightIntensity = L.intensity.xyz * ndotl;

	float att = Attenuate(d, L);
	lightIntensity *= att;

	float spotFactor = pow(max(dot(-lightVec, L.direction.xyz), 0.0f), L.spotPower);
	lightIntensity *= spotFactor;

	return BlinnPhong(lightIntensity, lightVec, normal, toEye, mat);
}
*/

float4 main(LightingPass_PixelInput input) : SV_TARGET
{
	/*
	float depth = depthBuffer.Sample(linearWrapSampler, input.texCoord).x;

	float3 normal = normals.Sample(linearWrapSampler, input.texCoord).xyz;

	Light l;
	l.intensity = float4(1.0f, 1.0f, 1.0f, 1.0f);
	l.direction = float3(1.0f, 0.0f, 0.0f);

	Material testMat;
	testMat.diffuseAlbedo = colors.Sample(linearWrapSampler, input.texCoord).xyzw;
	testMat.fresnelR0 = float3(0.05f, 0.05f, 0.05f);
	testMat.shininess = 2.0f;

	float2 ndcTex = 2.0f * input.texCoord - 1.0f;

	float4 worldPos = mul(float4(ndcTex, depth, 1.0f), viewProjInv);
	worldPos /= worldPos.w;

	float3 toEye = normalize(eyePos.xyz - worldPos.xyz);

	return float4(ComputeDirectionalLight(testMat, l.intensity, -l.direction, normal, toEye), 1.0f);*/

	return float4(ssaoBuffer.Sample(linearWrapSampler, input.texCoord).xyz, 1.0f);
}
