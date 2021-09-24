#pragma once
#include <DirectXMath.h>

struct CB_VS_vertexshader
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX viewProj;
	DirectX::XMFLOAT3 eyePos;
};

struct CB_PS_light
{
	DirectX::XMFLOAT3 ambientLightColor;
	float ambientLightStrength;
};