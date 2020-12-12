#pragma once
#include <vector>
#include <cstdlib>
#include <GameObjects/Entities/Entity.h>

class Level
{
public:
	void Initialize(unsigned int width, unsigned int height, ID3D12Device* device, ID3D12GraphicsCommandList* command_list);
	void AddEntity(const Entity& e);
	void Update();
	void Render();

	~Level();
private:
	std::vector<Entity> entities;
	int width = 0;
	int height = 0;
	float* height_map = nullptr;
	Mesh* map = nullptr; //Wrap in future
};