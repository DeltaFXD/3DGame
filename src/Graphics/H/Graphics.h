#pragma once
#include "Graphics/H/AdapterReader.h"

class Graphics
{
public:
	bool Initialize(HWND hwnd, int width, int height);
private:
	bool InitializeDirectX(HWND hwnd, int width, int height);

	wrl::ComPtr<ID3D12Device> device;
	wrl::ComPtr<ID3D12CommandQueue> command_queue;
	wrl::ComPtr<IDXGISwapChain> swapchain;
};