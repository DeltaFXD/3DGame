#include "PerlinNoise.h"

float PerlinNoise::Perlin(float x, float y)
{
	int x0 = (int)x;
	int x1 = x0 + 1;
	int y0 = (int)y;
	int y1 = y0 + 1;

	float wx = x - (float)x0;
	float wy = y - (float)y0;

	float n0, n1, ix0, ix1, value;

	n0 = DotGridGradient(x0, y0, x, y);
	n1 = DotGridGradient(x1, y0, x, y);
	ix0 = Interpolate(n0, n1, wx);

	n0 = DotGridGradient(x0, y1, x, y);
	n1 = DotGridGradient(x1, y1, x, y);
	ix1 = Interpolate(n0, n1, wx);

	value = Interpolate(ix0, ix1, wy);

	return value;
}

float PerlinNoise::Interpolate(float x1, float x2, float weight)
{
	return (x2 - x1) * weight + x1;
}

XMFLOAT2 PerlinNoise::RandomGradient(int x, int y)
{
	float detRand = 3256.f * sin(x * 34561.f + y * 163589.f + 2385.f) * cos(x * 41356.f + y * 652439.f + 1259.f);
	// sin^2(a) + cos^2(a) = 1
	return XMFLOAT2(cos(detRand), sin(detRand));
}

float PerlinNoise::DotGridGradient(int ix, int iy, float x, float y)
{
	XMFLOAT2 grad = RandomGradient(ix, iy);

	XMFLOAT2 dist;
	dist.x = x - (float)ix;
	dist.y = y - (float)iy;
	
	XMVECTOR dot = XMVector2Dot(XMLoadFloat2(&grad), XMLoadFloat2(&dist));

	float res;

	XMStoreFloat(&res, dot);

	return res;
}