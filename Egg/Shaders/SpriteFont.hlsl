#define SpriteStaticRS \
"RootFlags ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |" \
"            DENY_DOMAIN_SHADER_ROOT_ACCESS |" \
"            DENY_GEOMETRY_SHADER_ROOT_ACCESS |" \
"            DENY_HULL_SHADER_ROOT_ACCESS )," \
"DescriptorTable ( SRV(t0) ),"\
"CBV(b0), "\
"StaticSampler(s0,"\
"           filter = FILTER_ANISOTROPIC,"\
"           addressU = TEXTURE_ADDRESS_CLAMP,"\
"           addressV = TEXTURE_ADDRESS_CLAMP,"\
"           addressW = TEXTURE_ADDRESS_CLAMP,"\
"           visibility = SHADER_VISIBILITY_PIXEL )"

Texture2D<float4> spriteTexture : register(t0);
SamplerState samplerState : register(s0);


struct IAOutput
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD;
};

struct VSOutput {
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
};

cbuffer Parameters : register(b0)
{
    float4x4 matrixTransform;
};

[RootSignature(SpriteStaticRS)]
VSOutput SpriteVertexShader(IAOutput iao)
{
    VSOutput vso;
    vso.position = mul(float4(iao.position, 1.0f), matrixTransform);
    vso.color = iao.color;
    vso.texCoord = iao.texCoord;
    return vso;
}

[RootSignature(SpriteStaticRS)]
float4 SpritePixelShader(VSOutput vso) : SV_Target0
{
    return spriteTexture.Sample(samplerState, vso.texCoord);
   // float4 v = spriteTexture.Sample(samplerState, vso.texCoord);
   // return float4(v.xyz * v.a, 1.0f);
    //return float4(vso.texCoord, 0, 1);
}
