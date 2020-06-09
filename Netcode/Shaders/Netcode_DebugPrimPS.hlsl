struct VSOutput {
	float4 position : SV_Position;
	float3 color : COLOR;
};

float4 main(VSOutput vso) : SV_Target{

	return float4(vso.color, 1.0f);

}
