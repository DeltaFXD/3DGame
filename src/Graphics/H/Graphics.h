#pragma once
#include "Graphics/H/AdapterReader.h"
#include "Graphics/H/Shaders.h"
#include <d3dx12.h>
#include "Graphics/H/Vertex.h"
#include "Graphics/H/ConstantBufferTypes.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "DescriptorHeap.h"
#include "ResourceUploadBatch.h"
#include <GraphicsMemory.h>
#include <WICTextureLoader.h>
#include "Graphics/H/Camera.h"
#include "Utility/H/Timer.h"

class Graphics
{
public:
	bool Initialize(HWND hwnd, int width, int height);
	void Render();
	void Update();
	Camera camera;
private:
	bool InitializeDirectX(HWND hwnd);
	void InitializeScene();
	void CreateDescriptorHeaps();
	void InitPipelineState();
	void WaitForPreviousFrame();
	void PopulateCommandList();

	wrl::ComPtr<ID3D12Device> device;
	wrl::ComPtr<ID3D12CommandQueue> command_queue;
	wrl::ComPtr<ID3D12CommandAllocator> command_allocator;
	wrl::ComPtr<ID3D12GraphicsCommandList> command_list;
	wrl::ComPtr<IDXGISwapChain3> swapchain;
	wrl::ComPtr<ID3D12RootSignature> root_signature;
	wrl::ComPtr<ID3D12PipelineState> pipeline_state;
	//Resources
	wrl::ComPtr<ID3D12Resource> render_target_view[2]; //TEMP TODO: FIX
	//wrl::ComPtr<ID3D12Resource> texture;
	wrl::ComPtr<ID3D12Resource> m_texture;
	wrl::ComPtr<ID3D12Resource> depth_stencil;
	//wrl::ComPtr<ID3D12Resource> textureUploadHeap;
	//Descriptor Heaps
	wrl::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	wrl::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	wrl::ComPtr<ID3D12DescriptorHeap> cbvsrvHeap;
	//wrl::ComPtr<ID3D12DescriptorHeap> textHeap;

	//Debug mode
#ifdef _DEBUG
	wrl::ComPtr<ID3D12InfoQueue> info;
#endif
	UINT m_rtvDescriptorSize;
	UINT m_cbvsrvDescriptorSize;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	//Shaders
	wrl::ComPtr<ID3D12Resource> vertex_buffer;		//TODO: wrap vertex buffer
	wrl::ComPtr<ID3D12Resource> index_buffer;		//TODO: wrap index buffer
	wrl::ComPtr<ID3D12Resource> constant_buffer;	//TODO: wrap constant buffer
	CB_VS_vertexshader constantBufferData;
	UINT8* constantBufferDataBegin;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	VertexShader vertex_shader;
	PixelShader pixel_shader;

	//Fencing
	wrl::ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;
	HANDLE m_fenceEvent;
	UINT m_frameIndex;

	std::unique_ptr<DirectX::GraphicsMemory> graphicsMemory;
	/*std::unique_ptr<DirectX::DescriptorHeap> spriteHeap;
	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> spriteFont;*/
	int wWidth = 0;
	int wHeight = 0;
	Timer fpsTimer;
};