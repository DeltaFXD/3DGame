#include "Level.h"

void Level::Initialize(int width, int height, ID3D12Device* device, ID3D12GraphicsCommandList* command_list, Camera* camera)
{
	m_camera = camera;

	m_width = width;
	m_height = height;

	m_map = new Map(width, height);

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			m_chunks.push_back(new Chunk(x, y, device, command_list, m_map));
		}
	}

	Node* astar_map = new Node[m_map->GetMapWidth() * m_map->GetMapHeight()]();
	
	for (int x = 0; x < m_map->GetMapWidth(); x++)
	{
		for (int y = 0; y < m_map->GetMapHeight(); y++)
		{
			astar_map[x + y * m_map->GetMapWidth()].X = x;
			astar_map[x + y * m_map->GetMapWidth()].Y = y;
			astar_map[x + y * m_map->GetMapWidth()].Solid = m_map->IsSolid(x, y);
		}
	}

	AStar::Initialize(astar_map, m_map->GetMapWidth(), m_map->GetMapHeight());
}

Level::~Level()
{
	if (m_camera != nullptr) m_camera = nullptr;

	if (m_map != nullptr) delete m_map;
}

float Level::GetHeight(float x, float y)
{
	if (m_map == nullptr) return 0.0f;

	return m_map->GetHeight(x, y);
}

void Level::AddEntity(Entity* e)
{
	e->Initialize(this);
	m_entities.push_back(e);
}

void Level::Update()
{
	static int camX = 0;
	static int camY = 0;
	static float x = 0;
	static float y = 0;
	x = m_camera_x;
	y = m_camera_y;
	m_camera_x = m_camera->GetPositionFloat3().x;
	m_camera_y = m_camera->GetPositionFloat3().z;
	camX = static_cast<int>(m_camera_x);
	camY = static_cast<int>(m_camera_y);

	m_current_chunk_x = camX / Map::chunkSize;
	m_current_chunk_y = camY / Map::chunkSize;


	for (auto it = m_entities.begin(); it != m_entities.end(); it++)
	{
		it.operator*()->Update();
	}
}

void Level::ReleaseCreationResources()
{
	for (auto it = m_chunks.begin(); it != m_chunks.end(); it++)
	{
		it.operator*()->ReleaseUploadResources();
	}
}

void Level::RenderMap()
{
	for (auto it = m_chunks.begin(); it != m_chunks.end(); it++)
	{
		if (it.operator*()->IsWithin(m_current_chunk_x, m_current_chunk_y, 2))
		{
			it.operator*()->Render();
		}
	}
}

void Level::RenderEntities()
{
	for (auto it = m_entities.begin(); it != m_entities.end(); it++)
	{
		it.operator*()->Render();
	}
}