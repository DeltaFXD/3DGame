cbuffer lightBuffer : register(b2)
{
    float3 ambientLightColor;
    float ambientLightStrength;
}

struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float2 inTexCoord : TEXCOORD;
    uint select : SELECTOR;
};

Texture2D objTexture : TEXTURE : register(t0);
Texture2D terrainTexture[4] : TEXTURE : register(t1);
SamplerState objSamplerState : SAMPLER : register(s0);

float4 PS(PS_INPUT input) : SV_TARGET
{
    float3 sampleColor;
    /*if (input.inPosition.y > 10.0f)
    {
        sampleColor = terrainTexture[0].Sample(objSamplerState, input.inTexCoord);
    }
    else if (input.inPosition.y > 2.0f)
    {
        sampleColor = terrainTexture[1].Sample(objSamplerState, input.inTexCoord);
    }
    else if (input.inPosition.y > -2.5f)
    {
        sampleColor = terrainTexture[2].Sample(objSamplerState, input.inTexCoord);
    }
    else
    {
        sampleColor = terrainTexture[3].Sample(objSamplerState, input.inTexCoord);
    }*/
    
    sampleColor = terrainTexture[input.select].Sample(objSamplerState, input.inTexCoord);
    
    float3 ambientLight = ambientLightColor * ambientLightStrength;
    
    float3 finalColor = sampleColor * ambientLight;
    
    return float4(finalColor, 1.0f);
}