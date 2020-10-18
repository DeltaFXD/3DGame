#include "Graphics/H/Graphics.h"
#include "Utility/H/Config.h"

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	if (!InitializeDirectX(hwnd, width, height))
	{
		return false;
	}

	Load();

	return true;
}

void Graphics::Render()
{
	
	
}

bool Graphics::InitializeDirectX(HWND hwnd, int width, int height)
{
	wrl::ComPtr<IDXGIFactory4> factory;
	//Call IDXGIFactory1::Release once the factory is no longer required.
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)factory.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create IDXGIFactory");
		exit(-1);
	}
	wrl::ComPtr<IDXGIAdapter1> adapter;
	AdapterReader::GetHardwareAdapter(factory.Get(), adapter.GetAddressOf());

	hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)device.GetAddressOf());

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create ID3D12Device");
		exit(-1);
	}

	//Command queue describer
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	hr = device.Get()->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), (void**)command_queue.GetAddressOf());

	if (FAILED(hr))
	{
		if (hr == E_OUTOFMEMORY)
		{
			ErrorLogger::Log(hr, "Insufficient memory to create command queue.");
			exit(-1);
		}
		ErrorLogger::Log(hr, "Failed to create ID3D12CommandQueue.");
		exit(-1);
	}

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 scd = {};
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC1));

	scd.Width = width;
	scd.Height = height;
	scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.Stereo = FALSE;
	//Supported by every device
	//TODO: add config support
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = Config::GetBufferFrameCount();
	scd.Scaling = DXGI_SCALING_STRETCH;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//Fullscreen swap chain desc.
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC scfd = {};
	ZeroMemory(&scfd, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));

	scfd.RefreshRate.Numerator = 60;
	scfd.RefreshRate.Denominator = 1;
	scfd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scfd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scfd.Windowed = TRUE;

	hr = factory.Get()->CreateSwapChainForHwnd(command_queue.Get(), hwnd, &scd, &scfd, NULL, swapchain.GetAddressOf());

	if (FAILED(hr))
	{
		switch (hr)
		{
		case E_OUTOFMEMORY:
		{
			ErrorLogger::Log(hr, "Memory is unavailable for SwapChain.");
			break;
		}
		case DXGI_ERROR_INVALID_CALL:
		{
			ErrorLogger::Log(hr, "Failed to create SwapChain: invalid data.");
			break;
		}
		default:
			ErrorLogger::Log(hr, "Failed to create SwapChain.");
		}
		exit(-1);
	}

	//Factory no longer needed
	factory.Get()->Release();

	// Describe and create a render target view (RTV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = Config::GetBufferFrameCount();
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = device.Get()->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)heap_desc.GetAddressOf());

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create ID3D12DescriptorHeap");
		exit(-1);
	}

	//desc_size = device.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	hr = swapchain.Get()->GetBuffer(0, __uuidof(ID3D12Resource), (void**)render_target_view.GetAddressOf());

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to get back buffer.");
		exit(-1);
	}

	device.Get()->CreateRenderTargetView(render_target_view.Get(), nullptr, heap_desc.Get()->GetCPUDescriptorHandleForHeapStart());

	hr = device.Get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)command_allocator.GetAddressOf());

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create ID3D12CommandAllocator");
		exit(-1);
	}

	return true;
}

void Graphics::Load()
{

}