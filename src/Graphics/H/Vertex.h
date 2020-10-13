#pragma once
#include <DirectXMath.h>

struct Vertex
{
	Vertex() {}
	Vertex(float x, float y, float z) : pos(x, y, z) {}

	DirectX::XMFLOAT3 pos;
};