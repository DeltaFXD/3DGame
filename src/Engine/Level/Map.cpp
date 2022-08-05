#include "Map.h"

const int Map::chunkSize = 64;
const float Map::baseAmp = 5.0f;
const float Map::ampDecay = 0.9f;
const float Map::baseFreq = 100.0f;
const float Map::scale = 35.71f;

Map::Map(int size_x, int size_y)
{
	width = size_x * chunkSize;
	height = size_y * chunkSize;
	map = new MapData[static_cast<int64_t>(width) * height];

	Generate();
}

Map::~Map()
{
	delete map;
	map = nullptr;
}

int Map::GetMapWidth()
{
	return width;
}

int Map::GetMapHeight()
{
	return height;
}

float Map::GetHeight(float x, float y)
{
	if (x <= 0 || x > width || y <= 0 || y > height) return 0.0f;

	int x1 = static_cast<int>(x);
	int y1 = static_cast<int>(y);
	int x2 = x1 + 1;
	int y2 = y1 + 1;

	float q11 = map[x1 + y1 * width].height;
	float q12 = map[x1 + y2 * width].height;
	float q21 = map[x2 + y1 * width].height;
	float q22 = map[x2 + y2 * width].height;
	
	//Bilinear interpolation
	return ((x2 - x) * (q11 * (y2 - y) + q12 * (y - y1)) + (x - x1) * (q21 * (y2 - y) + q22 * (y - y1)));
}

float Map::GetHeight(int x, int y)
{
	if (x < 0 || x > width || y < 0 || y > height) return 0.0f;

	return map[x + y * width].height;
}

XMFLOAT3 Map::GetNormal(int x, int y)
{
	if (x < 0 || x > width || y < 0 || y > height) return XMFLOAT3(0.0f, 0.0f, 0.0f);

	float c = map[x + y * width].height;
	float w = 0.0f;
	float n = 0.0f;
	float e = 0.0f;
	float s = 0.0f;

	if (x != 0) s = map[(x - 1) + y * width].height;
	if (y != 0) w = map[x + (y - 1) * width].height;
	if (x != (width - 1)) n = map[(x + 1) + y * width].height;
	if (y != (height - 1)) e = map[x + (y + 1) * width].height;
	// NW N NE
	// W  C  E
	// SW S SE
	XMFLOAT3 normal_nw(w - c, 1.0f, c - n);
	XMFLOAT3 normal_ne(c - e, 1.0f, c - n);
	XMFLOAT3 normal_se(c - e, 1.0f, s - c);
	XMFLOAT3 normal_sw(w - c, 1.0f, s - c);

	XMFLOAT3 normal_avg(0.0f, 0.0f, 0.0f);

	if (y != 0 && x != (width - 1))
	{
		normal_avg.x += normal_nw.x;
		normal_avg.y += normal_nw.y;
		normal_avg.z += normal_nw.z;
	}

	if (y != (height - 1) && x != (width - 1))
	{
		normal_avg.x += normal_ne.x;
		normal_avg.y += normal_ne.y;
		normal_avg.z += normal_ne.z;
	}

	if (y != (height - 1) && x != 0)
	{
		normal_avg.x += normal_se.x;
		normal_avg.y += normal_se.y;
		normal_avg.z += normal_se.z;
	}

	if (y != 0 && x != 0)
	{
		normal_avg.x += normal_sw.x;
		normal_avg.y += normal_sw.y;
		normal_avg.z += normal_sw.z;
	}

	XMVECTOR unit_normal = XMVector3Normalize(XMLoadFloat3(&normal_avg));

	XMStoreFloat3(&normal_avg, unit_normal);

	return normal_avg;
}

bool Map::IsSolid(float x, float y)
{
	if (x < 0 || x > width || y < 0 || y > height) return true;

	int cx = static_cast<int>(x);
	int cy = static_cast<int>(y);

	return (map[cx + cy * width].flags & MapDataFlags::SOLID) ? true : false;
}

bool Map::IsPenetrable(float x, float y)
{
	if (x < 0 || x > width || y < 0 || y > height) return true;

	int cx = static_cast<int>(x);
	int cy = static_cast<int>(y);

	return (map[cx + cy * width].flags & MapDataFlags::PENETRABLE) ? true : false;
}

void Map::Generate()
{
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			float perlin = 0.0f;
			float amp = baseAmp;
			float freq = baseFreq;

			float norm = 0.0f;

			for (int d = 0; d < 20; d++)
			{
				float sX = x / scale * freq;
				float yX = y / scale * freq;
				perlin += ((PerlinNoise::Perlin(sX, yX) + 1.0f) / 2.0f * amp);
				norm += amp;
				amp *= ampDecay;
				freq *= 2.0f;
			}

			//perlin = perlin / norm;

			map[x + y * width].height = perlin;
			if (x == (width - 1) || y == (height - 1))
			{
				map[x + y * width].flags = (MapDataFlags::OUT_OF_MAP | MapDataFlags::SOLID);
			}
			else
			{
				map[x + y * width].flags = MapDataFlags::PENETRABLE;
			}
		}
	}
}