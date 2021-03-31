#pragma once
#include "Texture.h"

class TextureManager
{
public:
	TextureManager();

	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, ID3D12DescriptorHeap* heap, UINT start, UINT max);

	int CreateTexture();
	int CreateTexture(int width, int height,const void* data);

	void SetTexture(UINT id);

	void ReleaseUploadResources();
private:
	ID3D12Device* device = nullptr;
	ID3D12GraphicsCommandList* command_list = nullptr;
	ID3D12DescriptorHeap* heap = nullptr;
	UINT start = 0;
	UINT max = 0;
	UINT num_textures = 0;
	UINT cbvsrvDescriptorSize = 0;

	std::vector<Texture> textures;
};