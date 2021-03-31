#pragma once
#include "Graphics/Mesh.h"
#include "Map.h"

using namespace DirectX;

class Chunk
{
public:
	Chunk(int offsetX, int offsetY, ID3D12Device* device, ID3D12GraphicsCommandList* command_list, Map* map);
	~Chunk();
	void Render();
	bool IsWithin(int x, int y, int max_dist);
	void ReleaseUploadResources();
private:
	int x, y;
	Mesh* map_mesh = nullptr;
};