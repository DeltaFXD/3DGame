#include "Graphics/H/Graphics.h"
#include "Utility/H/Config.h"

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
	//Call IDXGIFactory1::Release once the factory is no longer required.
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)factory.GetAddressOf());
	if (hr != S_OK)
	{
		ErrorLogger::Log(hr, "Failed to creatre IDXGIFactory");
		exit(-1);
	}
	wrl::ComPtr<IDXGIAdapter1> adapter;
	AdapterReader::GetHardwareAdapter(factory.Get(), adapter.GetAddressOf());

	hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)device.GetAddressOf());

	if (hr != S_OK)
	{
		ErrorLogger::Log(hr, "Failed to creatre ID3D12Device");
		exit(-1);
	}

	//Command queue describer
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	hr = device.Get()->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), (void**)command_queue.GetAddressOf());

	if (hr != S_OK)
	{
		if (hr == E_OUTOFMEMORY)
		{
			ErrorLogger::Log(hr, "Insufficient memory to create command queue.");
			exit(-1);
		}
		ErrorLogger::Log(hr, "Failed to creatre ID3D12CommandQueue.");
		exit(-1);
	}

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = Config::GetBufferFrameCount();
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	hr = factory.Get()->CreateSwapChainForHwnd(command_queue.Get(), hwnd, &swapChainDesc, NULL, NULL, swapchain.GetAddressOf());

	if (hr != S_OK)
	{
		ErrorLogger::Log(hr, "Failed to creatre SwapChain.");
		exit(-1);
	}



	return true;
}