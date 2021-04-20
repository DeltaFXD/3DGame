#include "AStar.h"

bool AStar::s_initialized = false;
std::vector<Node*> AStar::s_open = std::vector<Node*>();
std::vector<Node*> AStar::s_closed = std::vector<Node*>();
Node* AStar::s_current = nullptr;
Node* AStar::s_map = nullptr;
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

void AStar::Initialize(Node* map, int mapWidth, int mapHeight)
{
	static AStar instance_;

	if (s_initialized == false && s_map == nullptr) return;

	s_map = map;
	s_mapWidth = mapWidth;
	s_mapHeight = mapHeight;

	s_initialized = true;

#ifdef _DEBUG //Debug mode
	//TODO: file logging
	std::string debugmsg = "AStar initialized.\n";
	OutputDebugStringA(debugmsg.c_str());
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

	if (!s_open.empty())
	{
		for (auto e : s_open)
		{
			e->G = 0.0;
			e->H = 0.0;
			e->Parent = nullptr;
		}
		s_open.clear();
	}

	if (!s_closed.empty())
	{
		for (auto e : s_closed)
		{
			e->G = 0.0;
			e->H = 0.0;
			e->Parent = nullptr;
		}
		s_closed.clear();
	}

	s_xStart = xStart;
	s_yStart = yStart;
	s_xEnd = xEnd;
	s_yEnd = yEnd;

	if (xEnd < 0 || xEnd > s_mapWidth || yEnd < 0 || yEnd > s_mapHeight)
	{
#ifdef _DEBUG //Debug mode
		std::string debugmsg = "Returning because destination out of the map.\n";
		OutputDebugStringA(debugmsg.c_str());
#endif
		return path;
	}
	else
	{
		if (s_map[(int)xEnd + (int)yEnd * s_mapWidth].Solid)
		{
#ifdef _DEBUG //Debug mode
			std::string debugmsg = "Returning because destination unreachable.\n";
			OutputDebugStringA(debugmsg.c_str());
#endif
			return path;
		}
	}

	s_current = s_map + (static_cast<int>(s_xStart) + static_cast<int>(s_yStart) * s_mapWidth);
	s_current->G = 0;
	s_current->H = 0;

	s_closed.push_back(s_current);
	AddNeigborsToOpenList();
	while (s_current->X != static_cast<int>(s_xEnd) || s_current->Y != static_cast<int>(s_yEnd))
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

	XMFLOAT2 p = XMFLOAT2(s_xEnd, s_yEnd);
	path.push_back(p);
	while (s_current->Parent != nullptr)
	{
		Node* prev = s_current;
		s_current = s_current->Parent;
		p.x = s_current->X + 0.5f;
		p.y = s_current->Y + 0.5f;
		path.push_back(p);
		prev->Parent = nullptr;
	}

	return path;
}

bool AStar::FindNeigborInList(bool open_close, Node* node)
{
	if (open_close)
	{
		for (auto e : s_open)
		{
			if (e == node) return true;
		}
	}
	else
	{
		for (auto e : s_closed)
		{
			if (e == node) return true;
		}
	}

	return false;
}

void AStar::AddNeigborsToOpenList()
{
	if (Distance(s_xStart, s_yStart) > 50.0) return;
	Node* node = nullptr;
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			if (x != 0 && y != 0)
			{
				if (s_current->X + x >= 0 && s_current->X + x < s_mapWidth && s_current->Y + y >= 0 && s_current->Y <= s_mapHeight)
				{
					int cX = s_current->X;
					int cY = s_current->Y;
					if (x == y)
					{
						if (s_map[cX + x + (cY + x - y) * s_mapWidth].Solid) continue;
						if (s_map[cX + x - y + (cY + y) * s_mapWidth].Solid) continue;
					}
					else
					{
						if (s_map[cX + x + (cY + x + y) * s_mapWidth].Solid) continue;
						if (s_map[cX + x + y + (cY + y) * s_mapWidth].Solid) continue;
					}
				}
				else
					continue;
			}
			node = s_map + (s_current->X + x + (s_current->Y + y) * s_mapWidth);
			node->Parent = s_current;
			node->G = s_current->G;
			node->H = Distance(x, y);
			if ((x != 0 || y != 0) //not current node
				&& s_current->X + x >= 0 && s_current->X + x < s_mapWidth //check boundaries
				&& s_current->Y + y >= 0 && s_current->Y + y < s_mapHeight
				&& !s_map[s_current->X + x + (s_current->Y + y) * s_mapWidth].Solid //check for node if it's walkable
				&& !FindNeigborInList(true, node) && !FindNeigborInList(false, node)) //if not already a node
			{
				node->G = node->Parent->G + 1.0;
				if (x != 0 && y != 0)
				{
					node->G += 1.0; //if it's diagonal movement add extra cost
				}
				s_open.push_back(node);
			}
		}
		std::sort(s_open.begin(), s_open.end(), Node::CompareNodeToNode);
		
	}
}

double AStar::Distance(float x, float y)
{
	return sqrt(pow(s_current->X + x - s_xEnd, 2) + pow(s_current->Y + y - s_yEnd, 2));
}