struct SpriteFont_PixelInput {
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
};

Texture2D<float4> spriteTexture : register(t0);
SamplerState samplerState : register(s0);


float4 main(SpriteFont_PixelInput input) : SV_Target0
{
    return spriteTexture.Sample(samplerState, input.texCoord);
}
