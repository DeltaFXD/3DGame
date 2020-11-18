#include "Texture.h"

Texture::Texture() : 
	m_gpu_addr((D3D12_GPU_VIRTUAL_ADDRESS)0),
	m_allocatedMemory(nullptr),
	m_useageState(D3D12_RESOURCE_STATE_COMMON),
	m_transitionState((D3D12_RESOURCE_STATES)-1)
{

}