#pragma once
#include "AdapterReader.h"
#include "Shaders.h"
#include <d3dx12.h>
#include <GraphicsMemory.h>
#include "Camera.h"
#include "Utility/Timer.h"
#include "GameObjects/GameObject.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "RenderItem.h"
#include "Engine/Level/Level.h"
#include <DXGIDebug.h>

enum class PSO_Types
{
	SOLID,
	TRANSLUCENT,
	WIREFRAME
};

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
	void ToggleFullscreen(HWND hwnd);
	//void AddRenderItemToRenderQueue(RenderItem item, PSO_Types type);

	int GetWidth();
	int GetHeight();
	bool GetTearingSupport();

	int tess = 1;

	Camera camera;
	//GameObject test_go;
	Level level;
	//Mesh* testT = nullptr;
	ModelManager model_mgr;
private:
	static const UINT FRAME_COUNT = 2;

	static Graphics* instance;
	static bool initialized;

	bool solid = true;
	bool m_vsync = true;
	bool m_allow_tearing = false;
	bool m_windowed_mode = true;

	RECT m_window_rect;

	Graphics(HWND hwnd, int width, int height);

	bool InitializeDirectX(HWND hwnd);
	void InitializeScene();
	void CreateRTV_DSV_DescriptorHeap();
	void CreateCBV_SRV_UAV_DescriptorHeap();
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
	wrl::ComPtr<ID3D12PipelineState> pipeline_state_wireframe;
	//Resources
	wrl::ComPtr<ID3D12Resource> render_target_view[FRAME_COUNT];
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
	ConstantBuffer<CB_VS_world> cb_world;
	ConstantBuffer<CB_VS_object> cb_object;
	CB_VS_world cb_world_data = CB_VS_world();
	CB_VS_object cb_object_data = CB_VS_object();
	wrl::ComPtr<ID3D12Resource> rootConstantBuffer;
	CB_PS_light rootConstantBufferData = CB_PS_light();

	std::unordered_map<std::string, wrl::ComPtr<ID3DBlob>> m_shaders;

	//Fencing
	wrl::ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue[FRAME_COUNT] = {0,0}; //Better way?
	HANDLE m_fenceEvent = nullptr;
	UINT m_frameIndex = 0;

	std::unique_ptr<DirectX::GraphicsMemory> graphicsMemory;

	int wWidth = 0;
	int wHeight = 0;
};