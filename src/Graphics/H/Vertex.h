#pragma once
#include <DirectXMath.h>

struct Vertex
{
	Vertex() : pos(0, 0, 0) {}
	Vertex(float x, float y, float z) : pos(x, y, z) {}

	DirectX::XMFLOAT3 pos;
};