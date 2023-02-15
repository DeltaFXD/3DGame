#ifndef VertexBuffer_h__
#define VertexBuffer_h__
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl/client.h>

namespace wrl = Microsoft::WRL;

template<class T>
class VertexBuffer
{
private:
	wrl::ComPtr<ID3D12Resource> buffer;
	wrl::ComPtr<ID3D12Resource> upload_buffer;
	D3D12_VERTEX_BUFFER_VIEW buffer_view;
public:
	VertexBuffer() {
	}

	VertexBuffer(const VertexBuffer<T>& rhs)
	{
		buffer = rhs.buffer;
		buffer_view = rhs.buffer_view;
	}

	~VertexBuffer()
	{
		buffer.Reset();
		upload_buffer.Reset();
	}

	D3D12_VERTEX_BUFFER_VIEW& Get()
	{
		return buffer_view;
	}

	//TODO: find better way really bad
	void FreeUploadResource()
	{
		upload_buffer.Reset();
	}

	HRESULT Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, T* data, UINT count)
	{
		HRESULT hr;
		const UINT buffer_size = sizeof(T) * count;
		const CD3DX12_HEAP_PROPERTIES buffer_heap_props(D3D12_HEAP_TYPE_DEFAULT);
		const CD3DX12_RESOURCE_DESC buffer_resource_desc = CD3DX12_RESOURCE_DESC::Buffer(buffer_size);

		hr = device->CreateCommittedResource(
			&buffer_heap_props,
			D3D12_HEAP_FLAG_NONE,
			&buffer_resource_desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr, __uuidof(ID3D12Resource), (void**)buffer.GetAddressOf());
		if (FAILED(hr))
			return hr;

		const CD3DX12_HEAP_PROPERTIES upload_buffer_heap_props(D3D12_HEAP_TYPE_UPLOAD);
		const CD3DX12_RESOURCE_DESC upload_buffer_resource_desc = CD3DX12_RESOURCE_DESC::Buffer(buffer_size);

		hr = device->CreateCommittedResource(
			&upload_buffer_heap_props,
			D3D12_HEAP_FLAG_NONE,
			&upload_buffer_resource_desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, __uuidof(ID3D12Resource), (void**)upload_buffer.GetAddressOf());
		if (FAILED(hr))
			return hr;

		// Copy data to the intermediate upload heap and then schedule a copy from the upload heap to the vertex buffer.
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = data;
		vertexData.RowPitch = buffer_size;
		vertexData.SlicePitch = buffer_size;

		UpdateSubresources<1>(command_list, buffer.Get(), upload_buffer.Get(), 0, 0, 1, &vertexData);

		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		command_list->ResourceBarrier(1, &barrier);

		// Initialize the vertex buffer view.
		buffer_view.BufferLocation = buffer->GetGPUVirtualAddress();
		buffer_view.StrideInBytes = sizeof(T);
		buffer_view.SizeInBytes = buffer_size;

		return hr;
	}
};

#endif // VertexBuffer_h__