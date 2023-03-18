#ifndef GPUResourceBuffer_h__
#define GPUResourceBuffer_h__
#include <d3d12.h>
#include <d3dx12.h>
#include "Utility/ErrorLogger.h"
#include <wrl/client.h>

namespace wrl = Microsoft::WRL;

template<class T>
class GPUResourceBuffer
{
public:
	GPUResourceBuffer(ID3D12Device* device, UINT count, bool isConstantBuffer)
	{
		if (isConstantBuffer)
		{
			m_data_size = static_cast<UINT>(sizeof(T) + (256 - sizeof(T) % 256));
		}
		else
		{
			m_data_size = sizeof(T);
		}

		//Create constant buffer
		const CD3DX12_HEAP_PROPERTIES buffer_heap_props(D3D12_HEAP_TYPE_UPLOAD);
		const CD3DX12_RESOURCE_DESC buffer_resource_desc = CD3DX12_RESOURCE_DESC::Buffer(static_cast<UINT64>(m_data_size) * count);

		HRESULT hr = device->CreateCommittedResource(
			&buffer_heap_props,
			D3D12_HEAP_FLAG_NONE,
			&buffer_resource_desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, __uuidof(ID3D12Resource), (void**)m_buffer.GetAddressOf()
		);
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, "Failed to create GPU resource buffer.");
			exit(-1);
		}
		
		// Map and initialize the constant buffer. We don't unmap this until the app closes. Keeping things mapped for the lifetime of the resource is okay.
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		hr = m_buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_buffer_data_begin));
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, "Failed to map GPU buffer.");
			exit(-1);
		}
	}

	GPUResourceBuffer(const GPUResourceBuffer& rhs) = delete;
	GPUResourceBuffer& operator=(const GPUResourceBuffer& rhs) = delete;

	~GPUResourceBuffer()
	{
		if (m_buffer != nullptr)
		{
			m_buffer->Unmap(0, nullptr);
		
			m_buffer_data_begin = nullptr;
		}
	}

	ID3D12Resource* Get() const
	{
		return m_buffer.Get();
	}

	UINT GetDataSize()
	{
		return m_data_size;
	}

	void CopyData(int index, const T& data)
	{
		memcpy(&m_buffer_data_begin[index * m_data_size], &data, sizeof(T));
	}
private:
	wrl::ComPtr<ID3D12Resource> m_buffer;
	UINT* m_buffer_data_begin = nullptr;

	UINT m_data_size = 0;
};
#endif