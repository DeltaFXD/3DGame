#include "Graphics.h"
#include "Utility/Config.h"

#define MATERIAL_COUNT 10
#define OBJECT_COUNT 1024

bool Graphics::initialized = false;
Graphics* Graphics::instance = nullptr;

//TODO: Legacy fullscreen, SwapChain->SetFullscreenState

Graphics::Graphics(HWND hwnd, int width, int height)
{
	initialized = true;

	//Set viewport
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = static_cast<float>(width);
	m_viewport.Height = static_cast<float>(height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	//Set scissor rect
	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = static_cast<LONG>(width);
	m_scissorRect.bottom = static_cast<LONG>(height);

	GetWindowRect(hwnd, &m_window_rect);

	m_cb_world_data.world = DirectX::XMMatrixIdentity();
	m_cb_world_data.viewProj = DirectX::XMMatrixIdentity();
	m_cb_world_data.proj = DirectX::XMMatrixIdentity();
	m_cb_world_data.eyePos = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_cb_default_object.localToWorld = DirectX::XMMatrixIdentity();
	m_cb_default_object.materialIndex = 0;

	wWidth = width;
	wHeight = height;
}

int Graphics::GetHeight()
{
	return wHeight;
}

int Graphics::GetWidth()
{
	return wWidth;
}

bool Graphics::GetTearingSupport()
{
	return m_allow_tearing;
}

Graphics* Graphics::Get()
{
	if (initialized)
	{
		return instance;
	}
	else
	{
		ErrorLogger::Log("Tried to access graphics object before initialization.");
		return nullptr;
	}
}

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	if (initialized) return false;

	instance = new Graphics(hwnd, width, height);

	if (!instance->InitializeDirectX(hwnd))
	{
		initialized = false;
		delete instance;
		return false;
	}

	instance->InitPipelineState();
	//Prepare scene
	instance->InitializeScene();

	return true;
}

void Graphics::MoveToNextFrame()
{
	HRESULT hr;

	const UINT64 m_fenceCurrentValue = m_fenceValue[m_frameIndex];
	//Signal and increment the fence value.
	hr = command_queue->Signal(m_fence.Get(), m_fenceCurrentValue);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to signal.");
		exit(-1);
	}

	m_frameIndex = swapchain->GetCurrentBackBufferIndex();

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < m_fenceValue[m_frameIndex])
	{
		hr = m_fence->SetEventOnCompletion(m_fenceValue[m_frameIndex], m_fenceEvent);
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, "Failed to wait for fence.");
			exit(-1);
		}

		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	// Set fence value for the next frame
	m_fenceValue[m_frameIndex] = m_fenceCurrentValue + 1;
}

void Graphics::WaitForGPU()
{
	HRESULT hr;

	// Schedule a signal command in queue
	hr = command_queue->Signal(m_fence.Get(), m_fenceValue[m_frameIndex]);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to signal.");
		exit(-1);
	}

	hr = m_fence->SetEventOnCompletion(m_fenceValue[m_frameIndex], m_fenceEvent);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to wait for fence.");
		exit(-1);
	}
	WaitForSingleObject(m_fenceEvent, INFINITE);

	// Increment fence value for current frame
	m_fenceValue[m_frameIndex]++;
}

void Graphics::ChangeFillMode(bool state)
{
	solid = state;
}

void Graphics::ToggleFullscreen(HWND hwnd)
{
	if (m_windowed_mode)
	{
		//Save current window rect
		GetWindowRect(hwnd, &m_window_rect);

		SetWindowLong(hwnd, GWL_STYLE, (WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU) & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

		RECT fullscreenWindowRect = {};
		try
		{
			if (swapchain)
			{
				HRESULT hr;
				wrl::ComPtr<IDXGIOutput> output;
				hr = swapchain->GetContainingOutput(&output);
				COM_ERROR_IF_FAILED(hr, "Failed to get output.");
				DXGI_OUTPUT_DESC desc;
				hr = output->GetDesc(&desc);
				COM_ERROR_IF_FAILED(hr, "Failed to get descriptor.");
				fullscreenWindowRect = desc.DesktopCoordinates;
			}
			else
			{
				COM_ERROR_IF_FAILED(S_FALSE, "");
			}
		}
		catch (COMException& e)
		{
			UNREFERENCED_PARAMETER(e);

			// Get primary display device
			DEVMODE devmode = {};
			devmode.dmSize = sizeof(DEVMODE);
			EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devmode);

			fullscreenWindowRect = {
				devmode.dmPosition.x,
				devmode.dmPosition.y,
				devmode.dmPosition.x + static_cast<LONG>(devmode.dmPelsWidth),
				devmode.dmPosition.y + static_cast<LONG>(devmode.dmPelsHeight)
			};
		}

		SetWindowPos(
			hwnd,
			HWND_TOPMOST,
			fullscreenWindowRect.left,
			fullscreenWindowRect.top,
			fullscreenWindowRect.right - fullscreenWindowRect.left,
			fullscreenWindowRect.bottom - fullscreenWindowRect.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(hwnd, SW_MAXIMIZE);
	}
	else
	{
		SetWindowLong(hwnd, GWL_STYLE, (WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU));

		SetWindowPos(
			hwnd,
			HWND_NOTOPMOST,
			m_window_rect.left,
			m_window_rect.top,
			m_window_rect.right - m_window_rect.left,
			m_window_rect.bottom - m_window_rect.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);

		ShowWindow(hwnd, SW_NORMAL);
	}

	m_windowed_mode = !m_windowed_mode;
}

void Graphics::Render()
{
	HRESULT hr;

	//Update constant buffer
	XMMATRIX world = XMMatrixIdentity();

	m_cb_world_data.world = XMMatrixTranspose(world);
	m_cb_world_data.proj = camera.GetProjectionMatrix();
	m_cb_world_data.viewProj = camera.GetViewMatrix() * camera.GetProjectionMatrix();
	m_cb_world_data.viewProj = XMMatrixTranspose(m_cb_world_data.viewProj);
	m_cb_world_data.eyePos = camera.GetPositionFloat3();

	m_scene_CB->CopyData(0, m_cb_world_data);

	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { command_list.Get() };
	command_queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	UINT presentFlags = (m_allow_tearing && m_windowed_mode) ? DXGI_PRESENT_ALLOW_TEARING : 0;

	// Present the frame.
	if (m_vsync)
	{
		hr = swapchain->Present(1, 0);
	}
	else
	{
		hr = swapchain->Present(0, presentFlags);
	}
	if (FAILED(hr))
	{
		hr = device->GetDeviceRemovedReason();
		ErrorLogger::Log(hr, "Failed to present frame.");
		exit(-1);
	}

	//https://github.com/Microsoft/DirectXTK/wiki/GraphicsMemory
	graphicsMemory->Commit(command_queue.Get());

	MoveToNextFrame();
}

void Graphics::Update()
{
	level.Update();
}

void Graphics::Destroy()
{
	if (!initialized) return;
	// Ensure that the GPU is no longer holding resources that are about to be cleaned up.
	instance->WaitForGPU();
	instance->MoveToNextFrame();
	instance->WaitForGPU();

	CloseHandle(instance->m_fenceEvent);

	/*if (instance->testT != nullptr)
	{
		delete instance->testT;
		instance->testT = nullptr;
	}*/

#ifdef _DEBUG
	HRESULT hr;
	IDXGIDebug1* debug = nullptr;
	hr = DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&debug);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to get debug interface.");
		exit(-1);
	}
	debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
	debug->Release();
#endif // _DEBUG

	initialized = false;
	delete instance;
}

bool Graphics::InitializeDirectX(HWND hwnd)
{
#pragma region D3Debug
	if (IsDebuggerPresent() == TRUE)
	{
#ifdef _DEBUG //Debug mode
		if (SUCCEEDED(D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)debug_controller.GetAddressOf())))
		{
			debug_controller.Get()->EnableDebugLayer();
		}
		else
		{
			ErrorLogger::Log("Failed to enable Debug Layer.");
		}
#endif
	}
	try {
		wrl::ComPtr<IDXGIFactory5> factory;
		//Call IDXGIFactory1::Release once the factory is no longer required.
		HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory5), (void**)factory.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create IDXGIFactory.");

		BOOL allow_tearing = FALSE;

		hr = factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing));

		m_allow_tearing = SUCCEEDED(hr) && allow_tearing;

		wrl::ComPtr<IDXGIAdapter1> adapter;
		AdapterReader::GetHardwareAdapter(factory.Get(), adapter.GetAddressOf());
		if (adapter.Get() == nullptr)
		{
			COM_ERROR_IF_FAILED(S_FALSE, "Couldn't find suitable adapter.");
		}

		hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)device.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create ID3D12Device.");

#ifdef _DEBUG //Debug mode
		hr = device->QueryInterface(__uuidof(ID3D12InfoQueue), (void**)info.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to get Info Queue.");

		info->PushEmptyStorageFilter();
		D3D12_INFO_QUEUE_FILTER_DESC filterDesc = { 0 };
		D3D12_MESSAGE_SEVERITY msList[] = { D3D12_MESSAGE_SEVERITY_INFO };
		filterDesc.NumCategories = 0;
		filterDesc.pCategoryList = nullptr;
		filterDesc.NumSeverities = 1;
		filterDesc.pSeverityList = msList;
		D3D12_INFO_QUEUE_FILTER filter = { 0 };
		filter.DenyList = filterDesc;
		hr = info->AddStorageFilterEntries(&filter);
		COM_ERROR_IF_FAILED(hr, "Failed to apply InfoQueue filter.");
#endif

		//Command queue describer
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		hr = device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), (void**)command_queue.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create ID3D12CommandQueue.");

		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC1 scd = { 0 };

		scd.Width = wWidth;
		scd.Height = wHeight;
		scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.Stereo = FALSE;

		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;

		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.BufferCount = FRAME_COUNT;
		scd.Scaling = DXGI_SCALING_STRETCH;
		scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		scd.Flags = m_allow_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		//Fullscreen swap chain desc.
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC scfd = { 0 };

		scfd.RefreshRate.Numerator = 60;
		scfd.RefreshRate.Denominator = 1;
		scfd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scfd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		scfd.Windowed = TRUE;

		IDXGISwapChain1* temp_swap_chain;
		hr = factory->CreateSwapChainForHwnd(command_queue.Get(), hwnd, &scd, &scfd, NULL, &temp_swap_chain);
		COM_ERROR_IF_FAILED(hr, "Failed to create SwapChain.");

		// Next upgrade the IDXGISwapChain1 to a IDXGISwapChain3 interface and store it.
		// This will allow us to use the newer functionality such as getting the current back buffer index.
		hr = temp_swap_chain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)swapchain.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to upgrade SwapChain.");

		temp_swap_chain = nullptr;

		//Disable default ALT + ENTER handling
		factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

		m_frameIndex = swapchain->GetCurrentBackBufferIndex();

		//Factory no longer needed
		factory->Release();

		//Create Descriptor Heaps
		CreateRTV_DSV_DescriptorHeap();
		
		CreateCBV_SRV_UAV_DescriptorHeap();

		m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_cbvsrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		//Create frame resources
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT i = 0; i < FRAME_COUNT; i++)
		{
			hr = swapchain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)render_target_view[i].GetAddressOf());
			COM_ERROR_IF_FAILED(hr, "Failed to get back buffer.");

			device->CreateRenderTargetView(render_target_view[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}

		// Create command allocators for each frame
		for (UINT i = 0; i < FRAME_COUNT; i++)
		{
			hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)command_allocator[i].GetAddressOf());
			COM_ERROR_IF_FAILED(hr, "Failed to create ID3D12CommandAllocator.");
		}

		graphicsMemory = std::make_unique<DirectX::GraphicsMemory>(device.Get());
	}
	catch (COMException& e)
	{
		ErrorLogger::Log(e);
		exit(-1);
	}
	return true;
}

void Graphics::CreateRTV_DSV_DescriptorHeap()
{
	try {
		HRESULT hr;
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FRAME_COUNT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		hr = device->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)rtvHeap.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create RTV Heap");

		// Describe and create a depth stencil view (DSV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		hr = device->CreateDescriptorHeap(&dsvHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)dsvHeap.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create DSV Heap");
	}
	catch (COMException& e)
	{
		ErrorLogger::Log(e);
		exit(-1);
	}
}

void Graphics::CreateCBV_SRV_UAV_DescriptorHeap()
{
	try {
		HRESULT hr;

		// Describe and create a shader resource view (SRV) and a constant buffer view (CBV)
		// Flags indicate that this descriptor heap can be bound to the pipeline and that descriptors contained in it can be referenced by a root table.
		D3D12_DESCRIPTOR_HEAP_DESC cbvsrvHeapDesc = {};
		cbvsrvHeapDesc.NumDescriptors = 64; //SRV + CBV
		cbvsrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvsrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		hr = device->CreateDescriptorHeap(&cbvsrvHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)cbvsrvHeap.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create SRV and CBV Heap");
	}
	catch (COMException& e)
	{
		ErrorLogger::Log(e);
		exit(-1);
	}
}

void Graphics::InitPipelineState()
{
	try {
		HRESULT hr;
		//Create root signature
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		//Set to highest root signature version
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		//Check for root signature version support
		hr = device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData));
		if (FAILED(hr))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 table{};
		table.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_ROOT_PARAMETER1 rootParameters[4] = {};
		rootParameters[0].InitAsConstantBufferView(0); //object
		rootParameters[1].InitAsShaderResourceView(0, 1); //material
		rootParameters[2].InitAsConstantBufferView(1); //world
		rootParameters[3].InitAsDescriptorTable(1, &table, D3D12_SHADER_VISIBILITY_PIXEL); //textures array

		//Create static sampler desc
		D3D12_STATIC_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.ShaderRegister = 0;

		//Create an empty root signature.
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rsd;
		rsd.Init_1_1(_countof(rootParameters), rootParameters, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		wrl::ComPtr<ID3DBlob> signature;
		wrl::ComPtr<ID3DBlob> error;

		hr = D3DX12SerializeVersionedRootSignature(&rsd, featureData.HighestVersion, &signature, &error);
		COM_ERROR_IF_FAILED(hr, (char*)error->GetBufferPointer());

		hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)root_signature.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create RootSignature.");

#ifdef _DEBUG //Debug mode
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		std::wstring shaderfolder = L"";
#pragma region DetermineShaderPath
		if (IsDebuggerPresent() == TRUE)
		{
#ifdef _DEBUG //Debug mode
#ifdef _WIN64 //x64
			shaderfolder = L"Shaders\\";
#else	//x86 (Win32)
			shaderfolder = L"Shaders\\";
#endif //Release Mode
#else
#ifdef _WIN64 //x64
			shaderfolder = L"Shaders\\";
#else	//x86 (Win32)
			shaderfolder = L"Shaders\\";
#endif
#endif 
		}

		std::wstring defaultShader = shaderfolder + L"Default.hlsl";

		const D3D_SHADER_MACRO textureArrayDefines[] =
		{
			"TEXTURE_COUNT", "1",
			NULL, NULL
		};

		m_shaders["defaultVS"] = Shader::CreateShader(defaultShader, textureArrayDefines, "VS_Main", ShaderType::VS, compileFlags);
		m_shaders["defaultPS"] = Shader::CreateShader(defaultShader, textureArrayDefines, "PS_Main", ShaderType::PS, compileFlags);

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		//Create Rasterizer State
		D3D12_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));

		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID; //SOLID / WIREFRAME
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;

		//DepthStencil
		D3D12_DEPTH_STENCIL_DESC depth_stencilDesc = { 0 };
		depth_stencilDesc.DepthEnable = TRUE;
		depth_stencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depth_stencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = root_signature.Get();
		psoDesc.VS = { reinterpret_cast<UINT8*>(m_shaders["defaultVS"]->GetBufferPointer()), m_shaders["defaultVS"]->GetBufferSize() };
		psoDesc.PS = { reinterpret_cast<UINT8*>(m_shaders["defaultPS"]->GetBufferPointer()), m_shaders["defaultPS"]->GetBufferSize() };
		psoDesc.RasterizerState = rasterizerDesc;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = depth_stencilDesc;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		hr = device->CreateGraphicsPipelineState(&psoDesc, __uuidof(ID3D12PipelineState), (void**)pipeline_state.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create PipelineState.");

		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

		hr = device->CreateGraphicsPipelineState(&psoDesc, __uuidof(ID3D12PipelineState), (void**)pipeline_state_wireframe.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create PipelineState.");

		hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator[m_frameIndex].Get(), pipeline_state.Get(), __uuidof(ID3D12GraphicsCommandList), (void**)command_list.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create CommandList.");
	}
	catch (COMException& e)
	{
		ErrorLogger::Log(e);
		exit(-1);
	}
 }

 void Graphics::InitializeScene()
 {
	 HRESULT hr;

	 try {
		 //Initialize texture manager
		 text_mgr.Initialize(device.Get(), command_list.Get(), cbvsrvHeap.Get(), 0, 1);

		 text_mgr.CreateTexture();

		 //testT = new Mesh(device.Get(), command_list.Get(), vertices, indices);

		 level.Initialize(4, 4, device.Get(), command_list.Get(), &camera);

		 //Create Scene ConstantBuffer
		 m_scene_CB = std::make_unique<GPUResourceBuffer<CB_Scene>>(device.Get(), 1, true);

		 m_scene_CB->CopyData(0, m_cb_world_data);

		 m_material = std::make_unique<GPUResourceBuffer<CB_Material>>(device.Get(), MATERIAL_COUNT, false);

		 m_object_CB = std::make_unique<GPUResourceBuffer<CB_Object>>(device.Get(), OBJECT_COUNT, true);

		 m_object_CB->CopyData(0, m_cb_default_object);

		 for (int i = 0; i < MATERIAL_COUNT; i++)
		 {
			 CB_Material default_data;
			 default_data.diffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			 default_data.freshnelR0 = XMFLOAT3(1.0f, 1.0f, 1.0f);
			 default_data.shininess = 1.0f;

			 m_material->CopyData(i, default_data);
		 }

		 //Needs constant buffer
		 /*if (!test_go.Initialize("Data\\Models\\female.obj", device.Get(), command_list.Get(), &cb_world))
			 exit(-1);*/

		 //test_go.AdjustPosition(1.0f, 0.0f, 1.0f);

		 //Create depth stencil view
		 D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		 depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		 depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		 depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		 D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		 depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		 depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		 depthOptimizedClearValue.DepthStencil.Stencil = 0;

		 const CD3DX12_HEAP_PROPERTIES dp_heap_props(D3D12_HEAP_TYPE_DEFAULT);
		 const CD3DX12_RESOURCE_DESC dp_resource_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D24_UNORM_S8_UINT, wWidth, wHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		 hr = device->CreateCommittedResource(
			 &dp_heap_props,
			 D3D12_HEAP_FLAG_NONE,
			 &dp_resource_desc,
			 D3D12_RESOURCE_STATE_DEPTH_WRITE,
			 &depthOptimizedClearValue,
			 __uuidof(ID3D12Resource),
			 (void**)depth_stencil.GetAddressOf()
		 );
		 COM_ERROR_IF_FAILED(hr, "Failed to create Stencil Buffer");

		 device->CreateDepthStencilView(depth_stencil.Get(), &depthStencilDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());

		 // Close the command list and execute it to begin the initial GPU setup.
		 hr = command_list->Close();
		 COM_ERROR_IF_FAILED(hr, "Failed to close command list to begin GPU setup.");

		 ID3D12CommandList* ppCommandLists[] = { command_list.Get() };
		 command_queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		 // Create synchronization objects and wait until assets have been uploaded to the GPU.
		 hr = device->CreateFence(m_fenceValue[m_frameIndex], D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)m_fence.GetAddressOf());
		 COM_ERROR_IF_FAILED(hr, "Failed to create synchronization object.");

		 m_fenceValue[m_frameIndex] = 1;

		 // Create an event handle to use for frame synchronization.
		 m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		 if (m_fenceEvent == nullptr)
		 {
			COM_ERROR_IF_FAILED(GetLastError(), "Failed to create event handle.");
		 }

		 
	 }
	 catch (COMException& e)
	 {
		 ErrorLogger::Log(e);
		 exit(-1);
	 }

	 WaitForGPU();

	 camera.SetPosition(5.0f, 0.0f, -5.0f);
	 //camera.SetLookAtPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
	 //camera.SetPosition(0.0f, 1.25f, -37.5f);
	 camera.SetRotation(0.0f, 0.0f, 0.0f);
	 camera.SetProjectionValues(90.0f, static_cast<float>(wWidth) / static_cast<float>(wHeight), 0.1f, 1000.0f);

	 text_mgr.ReleaseUploadResources();
	 level.ReleaseCreationResources();
	 //test_go.ReleaseCreationResources();
	 //testT->ReleaseLoadingResources();
 }

 void Graphics::PopulateCommandList()
 {
	 try {
		 HRESULT hr;
		 // Command list allocators can only be reset when the associated 
		 // command lists have finished execution on the GPU; apps should use 
		 // fences to determine GPU execution progress.
		 hr = command_allocator[m_frameIndex]->Reset();
		 COM_ERROR_IF_FAILED(hr, "Failed to Reset CommandAllocator.");

		 if (solid)
		 {
			 // However, when ExecuteCommandList() is called on a particular command 
			 // list, that command list can then be reset at any time and must be before re-recording.
			 hr = command_list->Reset(command_allocator[m_frameIndex].Get(), pipeline_state.Get());
			 COM_ERROR_IF_FAILED(hr, "Failed to Reset CommandList.");
		 }
		 else
		 {
			 hr = command_list->Reset(command_allocator[m_frameIndex].Get(), pipeline_state_wireframe.Get());
			 COM_ERROR_IF_FAILED(hr, "Failed to Reset CommandList.");
		 }

		 // Set necessary state.
		 command_list->SetGraphicsRootSignature(root_signature.Get());
		 ID3D12DescriptorHeap* ppHeaps[] = { cbvsrvHeap.Get() };
		 command_list->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		 command_list->SetGraphicsRootDescriptorTable(3, cbvsrvHeap->GetGPUDescriptorHandleForHeapStart());

		 auto worldCB = m_scene_CB->Get();
		 command_list->SetGraphicsRootConstantBufferView(2, worldCB->GetGPUVirtualAddress());

		 auto material = m_material->Get();
		 command_list->SetGraphicsRootShaderResourceView(1, material->GetGPUVirtualAddress());

		 auto objectDefaultCB = m_object_CB->Get();
		 command_list->SetGraphicsRootConstantBufferView(0, objectDefaultCB->GetGPUVirtualAddress());

		 command_list->RSSetViewports(1, &m_viewport);
		 command_list->RSSetScissorRects(1, &m_scissorRect);

		 // Indicate that the back buffer will be used as a render target.
		 const auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(render_target_view[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		 command_list->ResourceBarrier(1, &barrier1);

		 CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
		 CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());
		 command_list->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		 // Record commands.
		 const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		 command_list->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
		 command_list->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		 //Draw level
		 command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		 level.RenderMap();

		 //Draw entities on map
		 /*command_list->SetPipelineState(pipeline_state.Get());
		 command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		 level.RenderEntities();*/

		 //text_mgr.SetTexture(0);
		 /*CD3DX12_GPU_DESCRIPTOR_HANDLE cbvsrvHandle2(cbvsrvHeap->GetGPUDescriptorHandleForHeapStart(), 1, m_cbvsrvDescriptorSize);
		 command_list->SetGraphicsRootDescriptorTable(0, cbvsrvHandle2);
		 cbvsrvHandle.Offset(m_cbvsrvDescriptorSize);*/

		 //Draw model
		 //test_go.Render(camera.GetViewMatrix() * camera.GetProjectionMatrix());

		 //Draw text
		 //TODO: implement better text rendering https://www.braynzarsoft.net/viewtutorial/q16390-11-drawing-text-in-directx-12

		 // Indicate that the back buffer will now be used to present.
		 const auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(render_target_view[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		 command_list->ResourceBarrier(1, &barrier2);

		 hr = command_list->Close();
		 COM_ERROR_IF_FAILED(hr, "Failed to close CommandList.");
	 }
	 catch (COMException& e)
	 {
		 ErrorLogger::Log(e);
		 exit(-1);
	 }
 }
