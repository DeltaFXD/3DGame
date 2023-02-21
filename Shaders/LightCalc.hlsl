#define MaxLights 16

struct Light
{
    float3 strength;
    float lightPad1;
    float3 direction;
    float lightPad2;
    float3 position;
    float lightPad3;
};

struct Material
{
    float4 diffuseAlbedo;
    float3 freshnelR0;
    float shininess;
};