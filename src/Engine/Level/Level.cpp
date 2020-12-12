#include "Level.h"

void Level::Initialize(unsigned int width, unsigned int height, ID3D12Device* device, ID3D12GraphicsCommandList* command_list)
{
	if (height_map != nullptr)
		return;

	this->width = width;
	this->height = height;

	//TODO: improve
	height_map = new float[width * height];


}

void Level::AddEntity(const Entity& e)
{
	entities.push_back(e);
}

void Level::Update()
{
	for (auto it = entities.begin(); it != entities.end(); it++)
	{
		it->Update();
	}
}

void Level::Render()
{
	if (map != nullptr)
	{
		map->Render();
	}

	for (auto it = entities.begin(); it != entities.end(); it++)
	{
		it->Render();
	}
}

Level::~Level()
{
	if (map != nullptr)
	{
		delete map;
	}

	if (height_map != nullptr)
	{
		delete height_map;
	}
}