#include "Graphics/H/AdapterReader.h"

IDXGIAdapter1* AdapterReader::adapter = nullptr;

void AdapterReader::GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** pAdapter)
{
	if (adapter != nullptr)
	{
		*pAdapter = adapter;
		return;
	}

	for (UINT adapterIndex = 0; ; ++adapterIndex)
	{
		HRESULT h = pFactory->EnumAdapters1(adapterIndex, &adapter);
		if (DXGI_ERROR_NOT_FOUND == h)
		{
			if (adapterIndex == 0)
			{
				ErrorLogger::Log(h, "Missing Adapter");
				exit(-1);
			}
			//No more adapters to enumerate
			break;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
		{
			*pAdapter = adapter;
			return;
		}
	}
}