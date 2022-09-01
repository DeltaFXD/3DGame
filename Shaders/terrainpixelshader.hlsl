#include "LightCalc.hlsl"

struct PS_INPUT
{
    float4 inPosition : SV_POSITION;
    float3 inNormal : NORMAL;
    float2 inTexCoord : TEXCOORD;
    float col : COLOR;
    uint select : SELECTOR;
};

SamplerState objSamplerState : SAMPLER : register(s0);

float4 PS(PS_INPUT input) : SV_TARGET
{
    float3 sampleColor;
    
    input.inNormal = normalize(input.inNormal);

    float3 finalColor = float3(input.col, input.col, input.col);
    
    return float4(input.inNormal, 1.0f);
}