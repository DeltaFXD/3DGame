#include "Chunk.h"

Chunk::~Chunk()
{
	if (map_mesh != nullptr) delete map_mesh;
}

Chunk::Chunk(int offsetX, int offsetY, ID3D12Device* device, ID3D12GraphicsCommandList* command_list, Map* map) : x(offsetX), y(offsetY)
{
	int absX = offsetX * (Map::chunkSize - 1);
	int absY = offsetY * (Map::chunkSize - 1);

	this->x = offsetX;
	this->y = offsetY;

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
		x += 1;
		
		if (i % 2 == 0) {
			if ((i / Map::chunkSize) % 2 == 0)
			{
				vertex.texCoord = XMFLOAT2(0.0f, 0.0f);
			}
			else
			{
				vertex.texCoord = XMFLOAT2(0.0f, 1.0f);
			}
		}
		else
		{
			if ((i / Map::chunkSize) % 2 == 0)
			{
				vertex.texCoord = XMFLOAT2(1.0f, 0.0f);
			}
			else
			{
				vertex.texCoord = XMFLOAT2(1.0f, 1.0f);
			}
		}
		vertices.push_back(vertex);
	}

	int z = 0;
	int a = Map::chunkSize - 1;
	for (int i = 0; z < ((Map::chunkSize - 1) * (Map::chunkSize - 1) * 4); i++)
	{
		if (i != 0 && i % a == 0) {
			a += Map::chunkSize;
			continue;
		}
		/*indices.push_back(i);
		indices.push_back(i + Map::chunkSize);
		indices.push_back(i + Map::chunkSize + 1);

		indices.push_back(i);
		indices.push_back(i + Map::chunkSize + 1);
		indices.push_back(i + 1);
		z += 6;*/
		indices.push_back(i + Map::chunkSize + 1);
		indices.push_back(i + 1);
		indices.push_back(i + Map::chunkSize);
		indices.push_back(i);
		z += 4;
	}

	map_mesh = new Mesh(device, command_list, vertices, indices);
}

void Chunk::ReleaseUploadResources()
{
	map_mesh->ReleaseLoadingResources();
}

void Chunk::Render()
{
	if (map_mesh != nullptr) {
		map_mesh->Render();
	}
}

bool Chunk::IsWithin(int x, int y, int max_dist)
{
	//return (abs(this->x - x) + abs(this->y - y)) <= max_dist;
	return abs(this->x - x) <= max_dist && abs(this->y - y) <= max_dist;
}