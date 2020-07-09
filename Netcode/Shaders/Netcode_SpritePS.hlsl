struct SpriteFont_PixelInput {
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
};

Texture2D<float4> spriteTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer ControlDisplayData : register(b0) {
    float borderRadius;
    float borderWidth;
    float2 spriteSize;
    float2 screenSize;
    float2 screenPosition;
    float4 borderColor;
    int borderType;
    int backgroundType;
};

float4 SelectBackgroundColor(float2 texCoord, float4 albedoColor) {
    if(backgroundType == 1) {
        return albedoColor;
    }

    if(backgroundType == 2) {
        return spriteTexture.Sample(samplerState, texCoord) * albedoColor;
    }

    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 main(SpriteFont_PixelInput input) : SV_Target0
{
    float2 halfSize = spriteSize / 2.0f;
    float2 centeredTexCoord = 2.0f * input.texCoord - 1.0f;
    float2 foldedTexCoord = abs(centeredTexCoord);

    float2 offsettedEdge = float2(1.0f, 1.0f) - float2(borderRadius / halfSize.x, borderRadius / halfSize.y);

    if(foldedTexCoord.x > offsettedEdge.x || foldedTexCoord.y > offsettedEdge.y) {
        float2 projectedPoint = min(foldedTexCoord, offsettedEdge);

        float2 inverter = (halfSize - 0.5f);

        float2 p2 = projectedPoint * inverter;
        float2 p1 = foldedTexCoord * inverter;

        float dist = distance(p2, p1);

        if(dist < (borderRadius - borderWidth)) {
            return SelectBackgroundColor(input.texCoord, input.color);
        }

        if(dist < borderRadius) {
            return borderColor;
        }

        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    return SelectBackgroundColor(input.texCoord, input.color);
}
