#include "Level.h"

void Level::Initialize(int width, int height, ID3D12Device* device, ID3D12GraphicsCommandList* command_list, Camera* camera)
{
	this->camera = camera;

	this->width = width;
	this->height = height;

	map = new Map(width, height);

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			chunks.push_back(new Chunk(x, y, device, command_list, map));
		}
	}
}

Level::~Level()
{
	if (camera != nullptr) camera = nullptr;

	if (map != nullptr) delete map;
}

void Level::AddEntity(const Entity& e)
{
	entities.push_back(e);
}

void Level::Update()
{
	static int camX = 0;
	static int camY = 0;
	static float x = 0;
	static float y = 0;
	x = camera_x;
	y = camera_y;
	camera_x = camera->GetPositionFloat3().x;
	camera_y = camera->GetPositionFloat3().z;
	camX = static_cast<int>(camera_x);
	camY = static_cast<int>(camera_y);

	current_chunk_x = camX / Map::chunkSize;
	current_chunk_y = camY / Map::chunkSize;


	for (auto it = entities.begin(); it != entities.end(); it++)
	{
		it->Update();
	}
}

void Level::ReleaseCreationResources()
{
	for (auto it = chunks.begin(); it != chunks.end(); it++)
	{
		it.operator*()->ReleaseUploadResources();
	}
}

void Level::Render()
{
	for (auto it = chunks.begin(); it != chunks.end(); it++)
	{
		if (it.operator*()->IsWithin(current_chunk_x, current_chunk_y, 2))
		{
			it.operator*()->Render();
		}
	}

	for (auto it = entities.begin(); it != entities.end(); it++)
	{
		it->Render();
	}
}