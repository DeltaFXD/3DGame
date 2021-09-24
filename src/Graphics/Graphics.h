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
#include <dxgidebug.h>

class Graphics
{
public:
	Graphics(Graphics const&) = delete;
	void operator=(Graphics const&) = delete;

	static bool Initialize(HWND hwnd, int width, int height);
	static void Destroy();
	static Graphics* Get();

	void Render();
	void Update();
	void ChangeFillMode(bool state);
	int GetWidth();
	int GetHeight();

	Camera camera;
	GameObject test_go;
	Level level;
	Mesh* testT = nullptr;
private:
	static const UINT FRAME_COUNT = 2;

	static Graphics* instance;
	static bool initialized;

	bool solid = true;

	Graphics(HWND hwnd, int width, int height);

	bool InitializeDirectX(HWND hwnd);
	void InitializeScene();
	void CreateDescriptorHeaps();
	void InitPipelineState();
	void PopulateCommandList();
	void MoveToNextFrame();
	void WaitForGPU();

	wrl::ComPtr<ID3D12Device> device;
	wrl::ComPtr<ID3D12CommandQueue> command_queue;
	wrl::ComPtr<ID3D12CommandAllocator> command_allocator[FRAME_COUNT];
	wrl::ComPtr<ID3D12GraphicsCommandList> command_list;
	wrl::ComPtr<IDXGISwapChain3> swapchain;
	wrl::ComPtr<ID3D12RootSignature> root_signature;
	wrl::ComPtr<ID3D12PipelineState> pipeline_state;
	wrl::ComPtr<ID3D12PipelineState> pipeline_state_quad;
	wrl::ComPtr<ID3D12PipelineState> pipeline_state_wireframe;
	//Resources
	wrl::ComPtr<ID3D12Resource> render_target_view[FRAME_COUNT];
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
	wrl::ComPtr<ID3D12Debug> debug_controller;
#endif
	UINT m_rtvDescriptorSize = 0;
	UINT m_cbvsrvDescriptorSize = 0;

	D3D12_VIEWPORT m_viewport = D3D12_VIEWPORT();
	D3D12_RECT m_scissorRect = D3D12_RECT();

	TextureManager text_mgr;

	//Shaders
	ConstantBuffer<CB_VS_vertexshader> constantBuffer;
	CB_VS_vertexshader constantBufferData = CB_VS_vertexshader();
	wrl::ComPtr<ID3D12Resource> rootConstantBuffer;
	CB_PS_light rootConstantBufferData = CB_PS_light();

	Shader vertex_shader;
	Shader pixel_shader;

	Shader terrainVS;
	Shader terrainPS;
	Shader terrainDS;
	Shader terrainHS;

	//Fencing
	wrl::ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue[FRAME_COUNT] = {0,0}; //Better way?
	HANDLE m_fenceEvent = nullptr;
	UINT m_frameIndex = 0;

	std::unique_ptr<DirectX::GraphicsMemory> graphicsMemory;

	int wWidth = 0;
	int wHeight = 0;
};