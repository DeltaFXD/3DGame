#pragma once
#include "Graphics/H/AdapterReader.h"

class Graphics
{
public:
	bool Initialize(HWND hwnd, int width, int height);
	void Render();
private:
	bool InitializeDirectX(HWND hwnd, int width, int height);
	void Load();

	wrl::ComPtr<ID3D12Device> device;
	wrl::ComPtr<ID3D12CommandQueue> command_queue;
	wrl::ComPtr<IDXGISwapChain1> swapchain;
	wrl::ComPtr<ID3D12Resource> render_target_view;
	wrl::ComPtr<ID3D12CommandAllocator> command_allocator;
	wrl::ComPtr<ID3D12DescriptorHeap> heap_desc;
	wrl::ComPtr<ID3D12GraphicsCommandList> command_list;
};