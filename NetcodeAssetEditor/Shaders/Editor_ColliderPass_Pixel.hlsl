cbuffer ColliderData : register(b0) {
	float4x4 localTransform;
	float4 color;
	uint boneReference;
}

float4 main() : SV_TARGET
{
	return color;
}