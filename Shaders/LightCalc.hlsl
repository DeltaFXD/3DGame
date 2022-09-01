#define MaxLights 16

struct Light
{
    float3 strength;
    float3 direction;
    float3 position;
};

struct Material
{
    float4 diffuseAlbedo;
    float3 freshnelR0;
    float shininess;
};