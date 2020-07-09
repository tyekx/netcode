#define SpriteStaticRS \
"RootFlags ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |" \
"            DENY_DOMAIN_SHADER_ROOT_ACCESS |" \
"            DENY_GEOMETRY_SHADER_ROOT_ACCESS |" \
"            DENY_HULL_SHADER_ROOT_ACCESS )," \
"DescriptorTable ( SRV(t0, NumDescriptors = 1 ), Visibility = SHADER_VISIBILITY_PIXEL ),"\
"RootConstants(Num32BitConstants=14, b0, Visibility = SHADER_VISIBILITY_PIXEL ), "\
"RootConstants(Num32BitConstants=16, b1, Visibility = SHADER_VISIBILITY_VERTEX ), "\
"StaticSampler(s0,"\
"           filter = FILTER_ANISOTROPIC,"\
"           addressU = TEXTURE_ADDRESS_CLAMP,"\
"           addressV = TEXTURE_ADDRESS_CLAMP,"\
"           addressW = TEXTURE_ADDRESS_CLAMP,"\
"           Visibility = SHADER_VISIBILITY_PIXEL )"

struct SpriteFont_VertexInput
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD;
};

struct SpriteFont_VertexOutput {
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
};

cbuffer SpriteData : register(b1) {
    float4x4 matrixTransform;
}

[RootSignature(SpriteStaticRS)]
SpriteFont_VertexOutput main(SpriteFont_VertexInput iao)
{
    SpriteFont_VertexOutput vso;
    vso.position = mul(float4(iao.position, 1.0f), matrixTransform);
    vso.color = iao.color;
    vso.texCoord = iao.texCoord;
    return vso;
}
