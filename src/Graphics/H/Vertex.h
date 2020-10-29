#pragma once
#include <DirectXMath.h>

struct Vertex
{
	Vertex() : pos(0, 0, 0), color(0, 0, 0) {}
	Vertex(float x, float y, float z, float r, float g, float b) : pos(x, y, z), color(r, g, b) {}

	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 color;
};