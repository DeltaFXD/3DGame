#pragma once
#include <algorithm>
#include "Utility/PerlinNoise.h"
#include "MapData.h"

class Map 
{
public:
	Map(int size_x, int size_y);
	~Map();

	static const int chunkSize;
	static const float scale;

	float GetHeight(float x, float y);
	float GetHeight(int x, int y);
	bool IsSolid(float x, float y);
	bool IsPenetrable(float x, float y);
private:
	void Generate();

	int width;
	int height;
	MapData* map = nullptr;
};