#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <assimp/material.h>
#include "Utility/ErrorLogger.h"
#include "Color.h"

namespace wrl = Microsoft::WRL;

class Texture
{
public:
	Texture(D3D12_CPU_DESCRIPTOR_HANDLE Handle);

	HRESULT Create(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, UINT width, UINT height, const void* color, aiTextureType type);

	void Destroy();
	void ReleaseUploadResource();

	ID3D12Resource* GetTexture();
	const ID3D12Resource* GetTexture() const;
	aiTextureType GetType();

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const;
	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const;
private:
	wrl::ComPtr<ID3D12Resource> m_texture;
	wrl::ComPtr<ID3D12Resource> m_upload;
	D3D12_RESOURCE_STATES m_useageState;
	D3D12_GPU_VIRTUAL_ADDRESS m_gpu_addr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpu_handle;

	aiTextureType type = aiTextureType::aiTextureType_UNKNOWN;
};