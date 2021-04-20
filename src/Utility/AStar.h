#pragma once
#include <vector>
#include <algorithm>
#include <math.h>
#include <DirectXMath.h>
#include "Utility/ErrorLogger.h"
#include "Node.h"

using namespace DirectX;

class AStar
{
public:
	~AStar();
	static void Initialize(Node* map, int mapWidth, int mapHeight);
	static bool IsInitialized();
	static std::vector<XMFLOAT2> FindPath(float xStart, float yStart, float xEnd, float yEnd);

private:
	AStar();

	static bool FindNeigborInList(bool open_close, Node* node);
	static void AddNeigborsToOpenList();
	static double Distance(float x, float y);

	static bool s_initialized;

	static std::vector<Node*> s_open;
	static std::vector<Node*> s_closed;
	static Node* s_current;
	static Node* s_map;
	static int s_mapWidth, s_mapHeight;
	static float s_xStart, s_yStart;
	static float s_xEnd, s_yEnd;
};