#pragma once
#include <vector>
#include <cstdlib>
#include "GameObjects/Entities/Entity.h"
#include "Graphics/Camera.h"
#include "Chunk.h"
#include "Utility/AStar.h"

class Level
{
public:
	~Level();
	void Initialize(int width, int height, ID3D12Device* device, ID3D12GraphicsCommandList* command_list, Camera* camera);
	void AddEntity(Entity* e);
	void Update();
	void Render();
	void ReleaseCreationResources();
	float GetHeight(float x, float y);
private:
	std::vector<Entity*> entities;
	std::vector<Chunk*> chunks;

	Camera* camera = nullptr;
	Map* map = nullptr;

	int width = 0;
	int height = 0;
	int current_chunk_x = 0;
	int current_chunk_y = 0;
	float camera_x = 0.0f;
	float camera_y = 0.0f;
};