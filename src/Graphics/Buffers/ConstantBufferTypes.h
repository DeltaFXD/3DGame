#pragma once
#include <DirectXMath.h>

struct CB_Object
{
	DirectX::XMMATRIX localToWorld;
	UINT materialIndex;
	UINT objPad1;
	UINT objPad2;
	UINT objPad3;
};

struct CB_Scene
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX proj;
	DirectX::XMMATRIX viewProj;
	DirectX::XMFLOAT3 eyePos;
	float scenePad1;
};

struct CB_Light
{
	DirectX::XMFLOAT3 strength;
	float lightPad1;
	DirectX::XMFLOAT3 direction;
	float lightPad2;
	DirectX::XMFLOAT3 postion;
	float lightPad3;
};

struct CB_Material
{
	DirectX::XMFLOAT4 diffuseAlbedo;
	DirectX::XMFLOAT3 freshnelR0;
	float shininess;
};