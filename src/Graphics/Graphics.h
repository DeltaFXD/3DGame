#pragma once
#include "AdapterReader.h"
#include "Shaders.h"
#include <d3dx12.h>
#include "DescriptorHeap.h"
#include "ResourceUploadBatch.h"
#include <GraphicsMemory.h>
#include <WICTextureLoader.h>
#include "Camera.h"
#include "Utility/Timer.h"
#include "GameObjects/GameObject.h"
#include "TextureManager.h"
#include "Engine/Level/Level.h"

class Graphics
{
public:
	bool Initialize(HWND hwnd, int width, int height);
	void Render();
	void Update();
	void Destroy();
	Camera camera;
	GameObject test_go;
	Level level;
private:
	bool InitializeDirectX(HWND hwnd);
	void InitializeScene();
	void CreateDescriptorHeaps();
	void InitPipelineState();
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
	wrl::ComPtr<ID3D12Resource> m_texture;
	wrl::ComPtr<ID3D12Resource> m_texture2;
	wrl::ComPtr<ID3D12Resource> m_mat1;
	wrl::ComPtr<ID3D12Resource> m_mat2;
	wrl::ComPtr<ID3D12Resource> m_mat3;
	wrl::ComPtr<ID3D12Resource> m_mat4;
	wrl::ComPtr<ID3D12Resource> depth_stencil;
	//Descriptor Heaps
	wrl::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	wrl::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	wrl::ComPtr<ID3D12DescriptorHeap> cbvsrvHeap;

	//Debug mode
#ifdef _DEBUG
	wrl::ComPtr<ID3D12InfoQueue> info;
#endif
	UINT m_rtvDescriptorSize;
	UINT m_cbvsrvDescriptorSize;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	TextureManager text_mgr;

	//Shaders
	ConstantBuffer<CB_VS_vertexshader> constantBuffer;
	CB_VS_vertexshader constantBufferData;
	wrl::ComPtr<ID3D12Resource> rootConstantBuffer;
	CB_PS_light rootConstantBufferData;

	VertexShader vertex_shader;
	PixelShader pixel_shader;

	//Fencing
	wrl::ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;
	UINT64 m_fencePrevValue;
	HANDLE m_fenceEvent;
	UINT m_frameIndex;

	std::unique_ptr<DirectX::GraphicsMemory> graphicsMemory;

	int wWidth = 0;
	int wHeight = 0;
};