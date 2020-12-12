#pragma once
#include <DirectXMath.h>

using namespace DirectX;

class PerlinNoise
{
public:
	static float Perlin(float x, float y);
private:
	static float Interpolate(float x1, float x2, float weight);
	static XMFLOAT2 RandomGradient(int x, int y);
	static float DotGridGradient(int ix, int iy, float x, float y);
};