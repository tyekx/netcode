/*
Defines:
IAO_HAS_NORMAL
IAO_HAS_TEXCOORD
IAO_HAS_SKELETON
IAO_HAS_TANGENT_SPACE
IAO_HAS_COLOR
SHADER_NUM_LIGHTS
SHADER_CB_USE_PERMESH
SHADER_CB_USE_PEROBJECT
SHADER_CB_USE_BONEDATA
SHADER_CB_USE_PERFRAME
SHADER_TEX_NORMAL
SHADER_TEX_DIFFUSE
*/

struct IAOutput {
	float3 position : POSITION;

#ifdef IAO_HAS_NORMAL
	float3 normal : NORMAL;
#endif

#ifdef IAO_HAS_TEXCOORD
	float2 texCoord : TEXCOORD;
#endif

#ifdef IAO_HAS_SKELETON
#define SHADER_CB_USE_BONEDATA
	float3 weights : WEIGHTS;
	int4 boneIds : BONEIDS;
#endif

#ifdef IAO_HAS_TANGENT_SPACE
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
#endif

#ifdef IAO_HAS_COLOR
	float3 color : COLOR;
#endif
};


struct VSOutput {
	float4 position : SV_Position;

#ifdef SHADER_NUM_LIGHTS
	float4 worldPos : POSITION;
#endif

#ifdef IAO_HAS_NORMAL
	float3 normal : NORMAL;
	float3 viewDir : VIEWDIR;
#endif

#ifdef IAO_HAS_TEXCOORD
	float2 texCoord : TEXCOORD;
#endif

#ifdef IAO_HAS_COLOR
	float3 color : COLOR;
#endif
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

struct Material {
	float4 diffuse;
	float3 fresnelR0;
	float shininess;
};

#ifdef SHADER_CB_USE_PERMESH
cbuffer PerMeshCb : register(b0) {
	Material material;
};
#endif

#ifdef SHADER_CB_USE_PEROBJECT
cbuffer PerObjectCb : register(b1) {
	float4x4 model;
	float4x4 modelInv;
}
#endif

#ifdef SHADER_CB_USE_BONEDATA
cbuffer BoneDataCb : register(b2) {
	float4x4 bindTransforms[128];
	float4x4 toRootTransforms[128];
}
#endif

#ifdef SHADER_CB_USE_PERFRAME
cbuffer PerFrameCb : register(b3) {
	float4x4 viewProj;
	float4x4 viewProjInv;
	float4x4 view;
	float4x4 proj;
	float4 eyePosition;
	Light lights[16];
};
#endif

#ifdef SHADER_TEX_DIFFUSE
Texture2D diffuseTex : register(t0);
#endif

#ifdef SHADER_TEX_NORMAL
Texture2D normalTex : register(t1);
#endif

SamplerState linearWrapSampler : register(s0);

float Attenuate(float distanceToSource, Light l) {
	return saturate((l.falloffEnd - distanceToSource) / (l.falloffEnd - l.falloffStart));
}

float3 Fresnel(Material mat, float ndotl) {
	float f0 = 1.0f - ndotl;
	float3 R0 = mat.fresnelR0;
	return R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);
}

float3 BlinnPhong(float3 lightIntensity, float3 lightVec, float3 normalVec, float3 toEye, Material mat, float2 tex)
{
	const float m = mat.shininess * 256.0f;
	float3 halfVec = normalize(toEye + lightVec);
	float ndotl = dot(normalVec, lightVec);

	float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normalVec), 0.0f), m) / 8.0f;
	float3 fresnelFactor = Fresnel(mat, ndotl);

	float3 specAlbedo = fresnelFactor * roughnessFactor;

	specAlbedo = specAlbedo / (specAlbedo + 1.0f);

#if !defined(SHADER_TEX_DIFFUSE) || !defined(IAO_HAS_TEXCOORD)
	float3 diffuseAlbedo = mat.diffuse.rgb;
#endif

#ifdef SHADER_TEX_DIFFUSE
	float3 diffuseAlbedo = diffuseTex.Sample(linearWrapSampler, tex).xyz;
#endif


	return (diffuseAlbedo + specAlbedo) * lightIntensity;
}

float3 ComputeDirectionalLight(Light L, Material mat, float3 normal, float3 toEye, float2 tex)
{
	float3 toLight = L.position.xyz;
	float ndotl = max(dot(toLight, normal), 0.0f);

	return BlinnPhong(L.intensity.xyz * ndotl, toLight, normal, toEye, mat, tex);
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

VSOutput Vertex_Main(IAOutput iao) {
	VSOutput vso;
	
#ifdef IAO_HAS_SKELETON
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = iao.weights[0];
	weights[1] = iao.weights[1];
	weights[2] = iao.weights[2];
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 animPos = float3(0, 0, 0);
	float3 animNormal = float3(0, 0, 0);
#ifdef IAO_HAS_TANGENT_SPACE
	float3 animTangent = float3(0, 0, 0);
	float3 animBinormal = float3(0, 0, 0);
#endif

	for(int i = 0; i < 4; ++i) {
		animPos += weights[i] * (mul(float4(iao.position, 1), bindTransforms[iao.boneIds[i]]).xyz);
		animNormal += weights[i] * (mul(iao.normal, (float3x3)bindTransforms[iao.boneIds[i]]));
#ifdef IAO_HAS_TANGENT_SPACE
		animTangent += weights[i] * (mul(iao.tangent, (float3x3)bindTransforms[iao.boneIds[i]])));
		animBinormal += weights[i] * (mul(iao.binormal, (float3x3)bindTransforms[iao.boneIds[i]])));
#endif
	}

	iao.position = animPos;
	iao.normal = animNormal;
#ifdef IAO_HAS_TANGENT_SPACE
	iao.tangent = animTangent;
	iao.binormal = animBinormal;
#endif
	
#endif

	float4 worldPos = mul(float4(iao.position, 1), model);
	vso.position = mul(worldPos, viewProj);

#ifdef IAO_HAS_NORMAL
	vso.normal = mul(modelInv, float4(iao.normal, 0)).xyz;
	float4 hViewDir = eyePosition - worldPos;
	vso.viewDir = hViewDir.xyz / hViewDir.w;
#endif

#ifdef SHADER_NUM_LIGHTS
	vso.worldPos = float4(worldPos.xyz / worldPos.w, 1);
#endif

#ifdef IAO_HAS_TEXCOORD
	vso.texCoord = iao.texCoord;
#endif

#ifdef IAO_HAS_COLOR
	vso.color = iao.color;
#endif

	return vso;
}

float4 Pixel_Main(VSOutput vso) : SV_Target{

#ifdef IAO_HAS_TEXCOORD
	float2 texCoords = vso.texCoord;
#else 
	float2 texCorods = float2(0, 0);
#endif

#ifdef SHADER_NUM_LIGHTS
	float3 shade = ComputeDirectionalLight(
		lights[0], // todo, make iteration from this
		material,
		normalize(vso.normal),
		normalize(vso.viewDir).xyz,
		texCoords);

	return float4(shade, 1.0f);
#endif

#ifdef SHADER_TEX_DIFFUSE
	return diffuseTex.Sample(linearWrapSampler, vso.texCoord);
#endif

	return float4(material.diffuse);
}
