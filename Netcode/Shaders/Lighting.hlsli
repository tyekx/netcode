struct Light {
	float4 intensity;
	float4 position;
	float falloffStart;
	float falloffEnd;
	float spotPower;
	float __structPad0;
	float4 direction;
};

struct Material
{
	float4 DiffuseAlbedo;
	float3 FresnelR0;
	float Shininess;
};

float Attenuate(float d, Light l) {
	return saturate((l.falloffEnd - d) / (l.falloffEnd - l.falloffStart));
}

float3 Fresnel(Material mat, float ndotl) {
	float f0 = 1.0f - ndotl;
	float3 R0 = mat.FresnelR0;
	return R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);
}

float3 BlinnPhong(float3 lightIntensity, float3 lightVec, float3 normal, float3 toEye, Material mat)
{
	const float m = mat.Shininess * 256.0f;
	float3 halfVec = normalize(toEye + lightVec);
	float ndotl = dot(normal, lightVec);

	float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
	float3 fresnelFactor = Fresnel(mat, ndotl);

	float3 specAlbedo = fresnelFactor * roughnessFactor;

	specAlbedo = specAlbedo / (specAlbedo + 1.0f);

	return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightIntensity;
}

float3 ComputeDirectionalLight(Light L, Material mat, float3 normal, float3 toEye)
{
	float3 toLight = L.position.xyz;
	float ndotl = max(dot(toLight, normal), 0.0f);

	return BlinnPhong(L.intensity.xyz * ndotl, toLight, normal, toEye, mat);
}


float3 ComputePointLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
	// The vector from the surface to the light.
	float3 lightVec = L.position.xyz - pos;

	// The distance from surface to light.
	float d = length(lightVec);

	// Range test.
	if(d > L.falloffEnd)
		return 0.0f;

	// Normalize the light vector.
	lightVec /= d;

	// Scale light down by Lambert's cosine law.
	float ndotl = max(dot(lightVec, normal), 0.0f);
	float3 lightIntensity = L.intensity.xyz * ndotl;

	// Attenuate light by distance.
	float att = Attenuate(d, L);
	lightIntensity *= att;

	return BlinnPhong(lightIntensity, lightVec, normal, toEye, mat);
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
	// The vector from the surface to the light.
	float3 lightVec = L.position.xyz - pos;

	// The distance from surface to light.
	float d = length(lightVec);

	// Range test.
	if(d > L.falloffEnd)
		return 0.0f;

	// Normalize the light vector.
	lightVec /= d;

	// Scale light down by Lambert's cosine law.
	float ndotl = max(dot(lightVec, normal), 0.0f);
	float3 lightIntensity = L.intensity.xyz * ndotl;

	// Attenuate light by distance.
	float att = Attenuate(d, L);
	lightIntensity *= att;

	// Scale by spotlight
	float spotFactor = pow(max(dot(-lightVec, L.direction.xyz), 0.0f), L.spotPower);
	lightIntensity *= spotFactor;

	return BlinnPhong(lightIntensity, lightVec, normal, toEye, mat);
}
