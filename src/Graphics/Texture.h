#pragma once
#include <wrl/client.h>
#include <d3d12.h>

namespace wrl = Microsoft::WRL;

class Texture
{
public:
	Texture();

	void Create();

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const;

private:
	wrl::ComPtr<ID3D12Resource> m_texture;
	D3D12_RESOURCE_STATES m_useageState;
	D3D12_RESOURCE_STATES m_transitionState;
	D3D12_GPU_VIRTUAL_ADDRESS m_gpu_addr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpu_handle;

	// When using VirtualAlloc() to allocate memory directly, record the allocation here so that it can be freed.
	// The GpuVirtualAddress may be offset from the true allocation start.
	void* m_allocatedMemory;
};