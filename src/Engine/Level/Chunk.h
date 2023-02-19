#pragma once
#include "Graphics/Mesh.h"
#include "Map.h"

using namespace DirectX;

class Chunk
{
public:
	Chunk(int offsetX, int offsetY, ID3D12Device* device, ID3D12GraphicsCommandList* cmd_list, Map* map);
	~Chunk();
	void Render();
	bool IsWithin(int x, int y, int max_dist);
	void ReleaseUploadResources();
private:
	int m_x, m_y;
	Mesh* m_map_mesh = nullptr;
	ID3D12GraphicsCommandList* m_cmd_list = nullptr;
};