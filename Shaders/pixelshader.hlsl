cbuffer lightBuffer : register(b2)
{
    float3 ambientLightColor;
    float ambientLightStrength;
}

struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float3 inNormal : NORMAL;
    float2 inTexCoord : TEXCOORD;
    uint mode : MODE;
    uint select : SELECTOR;
};

Texture2D objTexture : TEXTURE : register(t0);
Texture2D terrainTexture[4] : TEXTURE : register(t1);
SamplerState objSamplerState : SAMPLER : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 sampleColor;
    if (input.mode == 0)
    {
        sampleColor = terrainTexture[input.select].Sample(objSamplerState, input.inTexCoord);
    }
    else
    {
        sampleColor = objTexture.Sample(objSamplerState, input.inTexCoord);
    }
    
    float3 ambientLight = ambientLightColor * ambientLightStrength;
    
    float3 finalColor = sampleColor * ambientLight;
    
    return float4(finalColor, 1.0f);
}