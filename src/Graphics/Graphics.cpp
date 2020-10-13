#include "Graphics/H/Graphics.h"

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	if (!InitializeDirectX(hwnd, width, height))
	{
		return false;
	}

	return true;
}

bool Graphics::InitializeDirectX(HWND hwnd, int width, int height)
{
	wrl::ComPtr<IDXGIFactory4> factory;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)factory.GetAddressOf());
	if (hr != S_OK)
	{
		ErrorLogger::Log(hr, "Failed to creatre IDXGIFactory");
	}
	IDXGIAdapter1* adapter;
	AdapterReader::GetHardwareAdapter(factory.Get(), &adapter);
	DXGI_ADAPTER_DESC1 desc;
	adapter->GetDesc1(&desc);


	return true;
}