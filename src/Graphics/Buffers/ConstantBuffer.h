#ifndef ConstantBuffer_h__
#define ConstantBuffer_h__
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl/client.h>

namespace wrl = Microsoft::WRL;

template<class T>
class ConstantBuffer
{
private:
	ConstantBuffer(const ConstantBuffer<T>& rhs);
private:
	// CB size is required to be 256-byte aligned.
	const UINT constantBufferSize = static_cast<UINT>(sizeof(T) + (256 - sizeof(T) % 256));
	UINT64 cbOffset = 0;
	wrl::ComPtr<ID3D12Resource> buffer;
	ID3D12GraphicsCommandList* command_list = nullptr;
	ID3D12Device* device = nullptr;
	ID3D12DescriptorHeap* heap = nullptr;
	UINT* bufferDataBegin = nullptr;
	UINT cbvsrvDescriptorSize = 0;
	UINT cbStart = 0;
public:
	ConstantBuffer() {}

	HRESULT Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, ID3D12DescriptorHeap* heap, UINT numView, UINT cbStart)
	{
		this->device = device;
		this->command_list = command_list;
		this->heap = heap;
		this->cbStart = cbStart;

		cbvsrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		//Create constant buffer
		HRESULT hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize * numView),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, __uuidof(ID3D12Resource), (void**)buffer.GetAddressOf()
		);
		if (FAILED(hr))
			return hr;

		cbOffset = 0;
		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvsrvHandle(heap->GetCPUDescriptorHandleForHeapStart(), cbStart, cbvsrvDescriptorSize);
		for (UINT i = 0; i < numView; i++)
		{
			//Describe and create a CBV
			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = buffer->GetGPUVirtualAddress() + cbOffset;
			cbvDesc.SizeInBytes = constantBufferSize;
			cbOffset += cbvDesc.SizeInBytes;
			device->CreateConstantBufferView(&cbvDesc, cbvsrvHandle);

			cbvsrvHandle.Offset(cbvsrvDescriptorSize);
		}

		// Map and initialize the constant buffer. We don't unmap this until the app closes. Keeping things mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		hr = buffer->Map(0, &readRange, reinterpret_cast<void**>(&bufferDataBegin));
		
		return hr;
	}

	void UpdateConstantBuffer(UINT slot, const T* data)
	{
		if (bufferDataBegin == nullptr)
			return;
		
		memcpy(bufferDataBegin + constantBufferSize * slot, data, sizeof(T));
	}

	void SetConstantBuffer(UINT slot)
	{
		CD3DX12_GPU_DESCRIPTOR_HANDLE cbvHandle(heap->GetGPUDescriptorHandleForHeapStart(), cbStart + slot, cbvsrvDescriptorSize);
		command_list->SetGraphicsRootDescriptorTable(1, cbvHandle);
	}
};

#endif // ConstantBuffer_h__