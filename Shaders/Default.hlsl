//Default Texture Count
#ifndef TEXTURE_COUNT
    #define TEXTURE_COUNT 64
#endif

#include "Shared.hlsl"

struct VS_INPUT
{
    float3 posLocal : POSITION;
    float3 normalLocal : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 posWorld : SV_POSITION;
    float3 normalWorld : NORMAL;
    float2 texCoord : TEXTCOORD;
};

VS_OUTPUT VS_Main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0.0f;
    
    float4 posW = mul(float4(input.normalLocal, 1.0f), localToWorld);
    
    output.posWorld = mul(posW, viewProj);
    
    output.normalWorld = mul(input.posLocal, (float3x3) localToWorld);
    
    output.texCoord = input.texCoord;
    
    return output;
}

float4 PS_Main(VS_OUTPUT input)
{
    MaterialData material = materialData[materialIndex];
    
    float4 diffuseColor = material.diffuseColor;
    uint diffuseIndex = material.diffuseMapIndex;
    
    diffuseColor *= textureMap[diffuseIndex].Sample(objSamplerState, input.texCoord);
    
    input.normalWorld = normalize(input.normalWorld);
    
    return diffuseColor;
}