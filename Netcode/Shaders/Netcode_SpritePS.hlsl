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
    int borderType;
    int backgroundType;
    float2 spriteSize;
    float2 screenSize;
    float2 screenPosition;
    //float2 texOffset;
    float4 borderColor;
    //float2x2 texTransform;
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
    if(borderType == 0) {
        return SelectBackgroundColor(input.texCoord, input.color);
    }

    const float sqrtf2 = 1.414213562f;

    float2 halfSize = spriteSize / 2.0f;
    float2 centeredTexCoord = 2.0f * input.texCoord - 1.0f;
    float2 foldedTexCoord = abs(centeredTexCoord);

    float br = max(borderRadius, borderWidth);

    float2 offsettedEdge = float2(1.0f, 1.0f) - float2(br / halfSize.x, br / halfSize.y);

    if(foldedTexCoord.x > offsettedEdge.x || foldedTexCoord.y > offsettedEdge.y) {
        float2 projectedPoint = min(foldedTexCoord, offsettedEdge);

        float2 inverter = (halfSize - 0.5f);

        float2 p2 = projectedPoint * inverter;
        float2 p1 = foldedTexCoord * inverter;

        float dist = distance(p2, p1);

        if(dist < (br - borderWidth)) {
            return SelectBackgroundColor(input.texCoord, input.color);
        }

        float maxAllowedDist = max(sqrtf2 * (borderWidth - borderRadius), borderRadius);

        if(dist < maxAllowedDist) {
            return borderColor;
        }

        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    return SelectBackgroundColor(input.texCoord, input.color);
}
