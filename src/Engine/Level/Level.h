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
	void RenderEntities();
	void RenderMap();
	void ReleaseCreationResources();
	float GetHeight(float x, float y);
private:
	std::vector<Entity*> m_entities;
	std::vector<Chunk*> m_chunks;

	Camera* m_camera = nullptr;
	Map* m_map = nullptr;

	int m_width = 0;
	int m_height = 0;
	int m_current_chunk_x = 0;
	int m_current_chunk_y = 0;
	float m_camera_x = 0.0f;
	float m_camera_y = 0.0f;
};