#include "Texture.h"

Texture::Texture(D3D12_CPU_DESCRIPTOR_HANDLE handle) :
	m_gpu_addr((D3D12_GPU_VIRTUAL_ADDRESS)0),
	m_useageState(D3D12_RESOURCE_STATE_COMMON),
	m_cpu_handle(handle)
{

}

HRESULT Texture::Create(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, UINT width, UINT height, const void* color, aiTextureType type)
{
	this->type = type;
	m_useageState = D3D12_RESOURCE_STATE_COPY_DEST;

	D3D12_RESOURCE_DESC textD = {};
	textD.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textD.Width = width;
	textD.Height = height;
	textD.DepthOrArraySize = 1;
	textD.MipLevels = 1;
	textD.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textD.SampleDesc.Count = 1;
	textD.SampleDesc.Quality = 0;
	textD.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	textD.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES heapP;
	heapP.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapP.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapP.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapP.CreationNodeMask = 1;
	heapP.VisibleNodeMask = 1;

	HRESULT hr = device->CreateCommittedResource(&heapP, D3D12_HEAP_FLAG_NONE, &textD, m_useageState, nullptr, __uuidof(ID3D12Resource), (void**)m_texture.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	D3D12_SUBRESOURCE_DATA textResource;
	textResource.pData = color;
	textResource.RowPitch = width * sizeof(Color);
	textResource.SlicePitch = textResource.RowPitch * height;

	//Upload to GPU
	UINT64 bufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);
	
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, __uuidof(ID3D12Resource), (void**)m_upload.GetAddressOf());
	if (FAILED(hr))
		return hr;

	UpdateSubresources(command_list, m_texture.Get(), m_upload.Get(), 0, 0, 1, &textResource);
	command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	//Create SRV
	device->CreateShaderResourceView(m_texture.Get(), nullptr, m_cpu_handle);

	return hr;
}

void Texture::Destroy()
{
	m_texture = nullptr;
	m_gpu_addr = (D3D12_GPU_VIRTUAL_ADDRESS)0;
	//Might not be the best practice TODO: find better solution
	m_cpu_handle.ptr = 0;
}

void Texture::ReleaseUploadResource()
{
	m_upload.Reset();
}

aiTextureType Texture::GetType()
{
	return type;
}

ID3D12Resource* Texture::GetTexture()
{
	return m_texture.Get();
}

const ID3D12Resource* Texture::GetTexture() const
{
	return m_texture.Get();
}

D3D12_GPU_VIRTUAL_ADDRESS Texture::GetGpuVirtualAddress() const
{
	return m_gpu_addr;
}

const D3D12_CPU_DESCRIPTOR_HANDLE& Texture::GetSRV() const
{
	return m_cpu_handle;
}