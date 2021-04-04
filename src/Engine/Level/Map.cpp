#include "Map.h"

const int Map::chunkSize = 64;
const float Map::scale = 35.71f;

Map::Map(int size_x, int size_y)
{
	width = size_x * chunkSize;
	height = size_y * chunkSize;
	map = new MapData[width * height];

	Generate();
}

Map::~Map()
{
	delete map;
	map = nullptr;
}

float Map::GetHeight(float x, float y)
{
	if (x < 0 || x > width || y < 0 || y > height) return 0.0f;

	int cx = static_cast<int>(x + 0.5f);
	int cy = static_cast<int>(y + 0.5f);

	float xh, yh;

	if (cx < x)
	{
		xh = map[cx + cy * width].height + (map[cx + 1 + cy * width].height - map[cx + cy * width].height) * (x - cx);
	}
	else
	{
		xh = map[cx + cy * width].height + (map[cx - 1 + cy * width].height - map[cx + cy * width].height) * (cx - x);
	}

	if (cy < y)
	{
		yh = map[cx + cy * width].height + (map[cx + (cy + 1) * width].height - map[cx + cy * width].height) * (y - cy);
	}
	else
	{
		yh = map[cx + cy * width].height + (map[cx + (cy - 1) * width].height - map[cx + cy * width].height) * (cy - y);
	}

	return (xh + yh) / 2.0f;
}

float Map::GetHeight(int x, int y)
{
	if (x < 0 || x > width || y < 0 || y > height) return 0;

	return map[x + y * width].height;
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
			float amp = 10.0f;
			float freq = 1.0f;
			for (int d = 0; d < 10; d++)
			{
				float sX = x / scale * freq;
				float yX = y / scale * freq;
				perlin += (PerlinNoise::Perlin(sX, yX) * amp);
				amp *= 0.5f;
				freq *= 2.0f;
			}
			perlin = std::max(-2.5f, perlin);
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