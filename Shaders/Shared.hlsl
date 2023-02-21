#include "LightCalc.hlsl"

//Default Texture Count
#ifndef TEXTURE_COUNT
    #define TEXTURE_COUNT 64
#endif

struct MaterialData
{
    float4 diffuseColor;
    uint diffuseMapIndex;
    uint normalMapIndex;
    uint materialPad1;
    uint materialPad2;
};

Texture2D textureMap[TEXTURE_COUNT] : register(t0);

StructuredBuffer<MaterialData> materialData : register(t0, space1);

SamplerState objSamplerState : SAMPLER : register(s0);

cbuffer objectData : register(b0)
{
    float4x4 localToWorld;
    uint materialIndex;
    uint objPad1;
    uint objPad2;
    uint objPad3;
};

cbuffer sceneData : register(b1)
{
    float4x4 view;
    float4x4 proj;
    float4x4 viewProj;
    float3 eyePos;
    float scenePad1;
};