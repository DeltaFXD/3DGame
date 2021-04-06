#include "AStar.h"

bool AStar::s_initialized = false;
std::vector<Node> AStar::s_open = std::vector<Node>();
std::vector<Node> AStar::s_closed = std::vector<Node>();
Node AStar::s_current = Node(nullptr, 0, 0, 0, 0);
int* AStar::s_map = nullptr;
int AStar::s_mapWidth = 0;
int AStar::s_mapHeight = 0;
float AStar::s_xStart = 0;
float AStar::s_yStart = 0;
float AStar::s_xEnd = 0;
float AStar::s_yEnd = 0;

AStar::AStar() {}

AStar::~AStar()
{
	if (s_map != nullptr)
	{
		delete s_map;
		s_map = nullptr;
	}
}

void AStar::Initialize(int* map, int mapWidth, int mapHeight)
{
	static AStar instance_;

	if (s_initialized == false && s_map == nullptr) return;

	s_map = map;
	s_mapWidth = mapWidth;
	s_mapHeight = mapHeight;

	s_initialized = true;

#ifdef _DEBUG //Debug mode
	//TODO: file logging
	std::cout << "AStar initialized.";
#endif
}

bool AStar::IsInitialized()
{
	return s_initialized;
}

std::vector<XMFLOAT2> AStar::FindPath(float xStart, float yStart, float xEnd, float yEnd)
{
	std::vector<XMFLOAT2> path = std::vector<XMFLOAT2>();

	if (!s_initialized) return path;

	s_open.clear();
	s_closed.clear();
	s_xStart = xStart;
	s_yStart = yStart;
	s_xEnd = xEnd;
	s_yEnd = yEnd;

	if (xEnd < 0 || xEnd > s_mapWidth || yEnd < 0 || yEnd > s_mapHeight)
	{
#ifdef _DEBUG //Debug mode
		std::cout << "Returning because destination out of the map.";
#endif
		return path;
	}
	else
	{
		if (s_map[(int)xEnd, (int)yEnd] == -1)
		{
#ifdef _DEBUG //Debug mode
			std::cout << "Returning because destination out of the map.";
#endif
			return path;
		}
	}

	s_current.X = s_xStart;
	s_current.Y = s_yStart;

	s_closed.push_back(s_current);
	AddNeigborsToOpenList();
	while (s_current.X != s_xEnd || s_current.Y != s_yEnd)
	{
		if (s_open.size() == 0)
		{
			return path;
		}
		s_current = s_open.back();
		s_open.pop_back();
		s_closed.push_back(s_current);
		AddNeigborsToOpenList();
	}
	//TODO: fix performance

}

bool AStar::FindNeigborInList(std::vector<Node> nodes, Node node)
{

}

void AStar::AddNeigborsToOpenList()
{

}

double AStar::Distance(int x, int y)
{

}