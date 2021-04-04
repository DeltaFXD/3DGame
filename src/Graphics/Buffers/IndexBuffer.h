#ifndef IndexBuffer_h__
#define IndexBuffer_h__
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl/client.h>

class IndexBuffer
{
private:
	IndexBuffer(const IndexBuffer& rhs);
private:
	wrl::ComPtr<ID3D12Resource> buffer;
	wrl::ComPtr<ID3D12Resource> upload_buffer;
	D3D12_INDEX_BUFFER_VIEW buffer_view;
	UINT count = 0;
public:
	IndexBuffer() {

	}

	~IndexBuffer()
	{
		buffer.Reset();
		upload_buffer.Reset();
	}

	//TODO: find better way really bad
	void FreeUploadResource()
	{
		upload_buffer.Reset();
	}

	UINT GetIndexCount() const
	{
		return count;
	}

	D3D12_INDEX_BUFFER_VIEW& Get()
	{
		return buffer_view;
	}

	HRESULT Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, DWORD* data, UINT count)
	{
		HRESULT hr;
		this->count = count;
		const UINT buffer_size = sizeof(DWORD) * count;

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

		// Copy data to the intermediate upload heap and then schedule a copy from the upload heap to the index buffer.
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = data;
		indexData.RowPitch = buffer_size;
		indexData.SlicePitch = buffer_size;

		UpdateSubresources<1>(command_list, buffer.Get(), upload_buffer.Get(), 0, 0, 1, &indexData);
		const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		command_list->ResourceBarrier(1, &barrier);

		//Initialize the index buffer view
		buffer_view.BufferLocation = buffer->GetGPUVirtualAddress();
		buffer_view.SizeInBytes = buffer_size;
		buffer_view.Format = DXGI_FORMAT_R32_UINT;

		return hr;
	}
};

#endif // !IndexBuffer_h__
