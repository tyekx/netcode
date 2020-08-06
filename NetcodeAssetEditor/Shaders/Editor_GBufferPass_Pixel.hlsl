struct GBufferPass_PixelInput {
	float4 position : SV_POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
};

cbuffer MaterialData : register(b3) {
	float4 diffuseColor;
	float4 ambientColor;
	float3 fresnelR0;
	float roughness;
	float2 tiles;
	int texturesFlags;
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

cbuffer LightingData : register(b4) {
	Light lights[4];
	int numLights;
};

Texture2D<float4> diffuseTexture : register(t0);
Texture2D<float3> normalTexture : register(t1);
Texture2D<float> roughnessTexture : register(t2);
Texture2D<float> ambientTexture : register(t3);
Texture2D<float> heightTexture : register(t4);

SamplerState linearWrapSampler : register(s0);

bool IsBitSet(int value, int nthBit) {
	return (value & (nthBit)) > 0;
}

#define DIFFUSE_TEXTURE 0
#define NORMAL_TEXTURE 1
#define AMBIENT_TEXTURE 2
#define SPECULAR_TEXTURE 3
#define ROUGHNESS_TEXTURE 4

float4 main(GBufferPass_PixelInput input) : SV_TARGET
{
	if(IsBitSet(texturesFlags, DIFFUSE_TEXTURE)) {
		return diffuseTexture.Sample(linearWrapSampler, input.texCoord) * diffuseColor;
	}

	return float4(input.normal, 1.0f) * diffuseColor;
}
