#include "TextureManager.h"

TextureManager::TextureManager() {}

void TextureManager::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, ID3D12DescriptorHeap* heap, UINT start, UINT max)
{
	this->device = device;
	this->command_list = command_list;
	this->heap = heap;
	this->start = start;
	this->max = max;

	cbvsrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

int TextureManager::CreateTexture()
{
	if (num_textures < (max - start))
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(heap->GetCPUDescriptorHandleForHeapStart(), start + num_textures, cbvsrvDescriptorSize);

		Texture texture(handle);

		texture.Create(device, command_list, 1, 1, Color(255, 255, 0), aiTextureType_DIFFUSE);

		textures.push_back(texture);

		num_textures++;
		return num_textures - 1;
	}
	else
	{
		return -1;
	}
}

void TextureManager::SetTexture(UINT id)
{
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbvsrvHandle(heap->GetGPUDescriptorHandleForHeapStart(), start + id, cbvsrvDescriptorSize);
	command_list->SetGraphicsRootDescriptorTable(0, cbvsrvHandle);
	cbvsrvHandle.Offset(cbvsrvDescriptorSize);
}

void TextureManager::ReleaseUploadResources()
{
	for (int i = 0; i < textures.size(); i++)
	{
		textures[i].ReleaseUploadResource();
	}
}