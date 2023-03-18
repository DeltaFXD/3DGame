#include "Chunk.h"

Chunk::~Chunk()
{
	m_cmd_list = nullptr;

	if (m_map_mesh != nullptr) delete m_map_mesh;
}

Chunk::Chunk(int offsetX, int offsetY, ID3D12Device* device, ID3D12GraphicsCommandList* cmd_list, Map* map) : m_x(offsetX), m_y(offsetY)
{
	m_cmd_list = cmd_list;

	int absX = offsetX * (Map::chunkSize - 1);
	int absY = offsetY * (Map::chunkSize - 1);

	std::vector<Vertex> vertices;
	std::vector<DWORD> indices;

	int x = 0;
	int y = 0;
	for (int i = 0; i < Map::chunkSize * Map::chunkSize; i++)
	{
		Vertex vertex;
		if (i != 0 && i % Map::chunkSize == 0)
		{
			y += 1;
			x = 0;
		}
		//vertex.pos = XMFLOAT3((float)(x + absX), 1.0f, (float)(y + absY));
		vertex.pos = XMFLOAT3((float)(x + absX), map->GetHeight(absX + x, absY + y), (float)(y + absY));
		vertex.normal = map->GetNormal(absX + x, absY + y);
		vertex.texCoord = XMFLOAT2(static_cast<float>(x), static_cast<float>(y));

		vertices.push_back(vertex);
		x += 1;
	}

	int z = 0;
	int a = Map::chunkSize - 1;
	for (int i = 0; z < ((Map::chunkSize - 1) * (Map::chunkSize - 1) * 6); i++)
	{
		if (i != 0 && i % a == 0) {
			a += Map::chunkSize;
			continue;
		}
		indices.push_back(i);
		indices.push_back(i + Map::chunkSize);
		indices.push_back(i + Map::chunkSize + 1);

		indices.push_back(i);
		indices.push_back(i + Map::chunkSize + 1);
		indices.push_back(i + 1);
		z += 6;
		/*indices.push_back(i + Map::chunkSize + 1);
		indices.push_back(i + 1);
		indices.push_back(i + Map::chunkSize);
		indices.push_back(i);
		z += 4;*/
	}

	m_map_mesh = new Mesh(device, m_cmd_list, vertices, indices);
}

void Chunk::ReleaseUploadResources()
{
	m_map_mesh->ReleaseLoadingResources();
}

void Chunk::Render()
{
	if (m_map_mesh != nullptr && m_cmd_list != nullptr) {
		m_cmd_list->IASetVertexBuffers(0, 1, &m_map_mesh->GetVertexBufferView());
		m_cmd_list->IASetIndexBuffer(&m_map_mesh->GetIndexBufferView());

		m_cmd_list->DrawIndexedInstanced(m_map_mesh->GetIndexCount(), 1, 0, 0, 0);
	}
}

bool Chunk::IsWithin(int x, int y, int max_dist)
{
	//return (abs(this->x - x) + abs(this->y - y)) <= max_dist;
	return abs(m_x - x) <= max_dist && abs(m_y - y) <= max_dist;
}