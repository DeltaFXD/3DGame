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
    float col : COLOR;
    uint select : SELECTOR;
};

Texture2D objTexture : TEXTURE : register(t0);
Texture2D terrainTexture[4] : TEXTURE : register(t1);
SamplerState objSamplerState : SAMPLER : register(s0);

float4 PS(PS_INPUT input) : SV_TARGET
{
    float3 sampleColor;
    
    input.inNormal = normalize(input.inNormal);
    
    /*sampleColor = terrainTexture[input.select].Sample(objSamplerState, input.inTexCoord);
    
    float3 ambientLight = ambientLightColor * ambientLightStrength;
    
    float3 finalColor = sampleColor * ambientLight;*/

    float3 finalColor = float3(input.col, input.col, input.col);
    
    return float4(input.inNormal, 1.0f);
}