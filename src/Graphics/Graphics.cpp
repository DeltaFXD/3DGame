#include "Graphics.h"
#include "Utility/Config.h"

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

	cb_world_data.viewProj = DirectX::XMMatrixIdentity();
	cb_world_data.eyePos = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	cb_object_data.world = DirectX::XMMatrixIdentity();

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

	cb_world_data.world = XMMatrixTranspose(world);
	cb_world_data.viewProj = camera.GetViewMatrix() * camera.GetProjectionMatrix();;
	cb_world_data.viewProj = XMMatrixTranspose(cb_world_data.viewProj);
	cb_world_data.eyePos = camera.GetPositionFloat3();

	cb_world.UpdateConstantBuffer(0, cb_world_data);

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

	if (instance->testT != nullptr)
	{
		delete instance->testT;
		instance->testT = nullptr;
	}

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
		cbvsrvHeapDesc.NumDescriptors = 256; //SRV + CBV
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

		CD3DX12_DESCRIPTOR_RANGE1 ranges[3] = {};
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		//https://docs.microsoft.com/en-us/windows/win32/direct3d12/creating-a-root-signature#code-for-defining-a-version-11-root-signature
		CD3DX12_ROOT_PARAMETER1 rootParameters[6] = {};
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[2].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[3].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[4].InitAsConstants(1, 1, 0, D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[5].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);

		//TODO: Dynamic sampler example https://www.programmersought.com/article/283396314/
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
		COM_ERROR_IF_FAILED(hr, "Failed to SerializeRootSignature.");

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

		std::wstring vertexPath = shaderfolder + L"vertexshader.hlsl";
		if (!vertex_shader.Initialize(vertexPath, "main", ShaderType::VS, compileFlags))
		{
			exit(-1);
		}
		std::wstring pixelPath = shaderfolder + L"pixelshader.hlsl";
		if (!pixel_shader.Initialize(pixelPath, "main", ShaderType::PS, compileFlags))
		{
			exit(-1);
		}

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
		psoDesc.VS = { reinterpret_cast<UINT8*>(vertex_shader.GeBuffer()->GetBufferPointer()), vertex_shader.GeBuffer()->GetBufferSize() };
		psoDesc.PS = { reinterpret_cast<UINT8*>(pixel_shader.GeBuffer()->GetBufferPointer()), pixel_shader.GeBuffer()->GetBufferSize() };
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

		
		
		std::wstring terrainPath = shaderfolder + L"terrainTessellation.hlsl";
		if (!terrainVS.Initialize(terrainPath, "VS", ShaderType::VS, compileFlags))
		{
			exit(-1);
		}
		if (!terrainHS.Initialize(terrainPath, "HS", ShaderType::HS, compileFlags))
		{
			exit(-1);
		}
		if (!terrainDS.Initialize(terrainPath, "DS", ShaderType::DS, compileFlags))
		{
			exit(-1);
		}
		std::wstring terrainPathPS = shaderfolder + L"terrainpixelshader.hlsl";
		if (!terrainPS.Initialize(terrainPathPS, "PS", ShaderType::PS, compileFlags))
		{
			exit(-1);
		}
		
		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC terrain_IED[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		//Create Rasterizer State
		D3D12_RASTERIZER_DESC rasterizerDesc2;
		ZeroMemory(&rasterizerDesc2, sizeof(D3D12_RASTERIZER_DESC));

		rasterizerDesc2.FillMode = D3D12_FILL_MODE_SOLID; //SOLID / WIREFRAME
		rasterizerDesc2.CullMode = D3D12_CULL_MODE_BACK;

		//DepthStencil
		D3D12_DEPTH_STENCIL_DESC depth_stencilDesc2 = { 0 };
		depth_stencilDesc2.DepthEnable = TRUE;
		depth_stencilDesc2.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depth_stencilDesc2.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC quad_psoDesc = {};
		quad_psoDesc.InputLayout = { terrain_IED, _countof(terrain_IED) };
		quad_psoDesc.pRootSignature = root_signature.Get();
		quad_psoDesc.VS = { reinterpret_cast<UINT8*>(terrainVS.GeBuffer()->GetBufferPointer()), terrainVS.GeBuffer()->GetBufferSize() };
		quad_psoDesc.HS = { reinterpret_cast<UINT8*>(terrainHS.GeBuffer()->GetBufferPointer()), terrainHS.GeBuffer()->GetBufferSize() };
		quad_psoDesc.DS = { reinterpret_cast<UINT8*>(terrainDS.GeBuffer()->GetBufferPointer()), terrainDS.GeBuffer()->GetBufferSize() };
		quad_psoDesc.PS = { reinterpret_cast<UINT8*>(terrainPS.GeBuffer()->GetBufferPointer()), terrainPS.GeBuffer()->GetBufferSize() };
		quad_psoDesc.RasterizerState = rasterizerDesc2;
		quad_psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		quad_psoDesc.DepthStencilState = depth_stencilDesc2;
		quad_psoDesc.SampleMask = UINT_MAX;
		quad_psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
		quad_psoDesc.NumRenderTargets = 1;
		quad_psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		quad_psoDesc.SampleDesc.Count = 1;
		quad_psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		hr = device->CreateGraphicsPipelineState(&quad_psoDesc, __uuidof(ID3D12PipelineState), (void**)pipeline_state_quad.GetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create PipelineState with Quad topology.");

		quad_psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

		hr = device->CreateGraphicsPipelineState(&quad_psoDesc, __uuidof(ID3D12PipelineState), (void**)pipeline_state_wireframe.GetAddressOf());
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

	 wrl::ComPtr<ID3D12Resource> uploadBuffer;
	 std::vector<Vertex> vertices;
	 std::vector<DWORD> indices;

	 vertices.push_back(Vertex(0.0f, 1.0f, 0.0f, -10.0f, -100.0f, -20.0f, 0.0f, 0.0f));
	 vertices.push_back(Vertex(10.0f, 0.0f, 0.0f, -100.0f, 100.0f, 20.0f, 0.0f, 0.0f));
	 vertices.push_back(Vertex(0.0f, -1.0f, 10.0f, -20.0f, -100.0f, -20.0f, 0.0f, 0.0f));
	 vertices.push_back(Vertex(10.0f, -2.0f, 10.0f, 10.0f, 100.0f, 20.0f, 0.0f, 0.0f));

	 indices.push_back(2);
	 indices.push_back(3);
	 indices.push_back(0);
	 indices.push_back(1);

	 try {
		 // Describe and create a SRV for the texture.
		 CD3DX12_CPU_DESCRIPTOR_HANDLE cbvsrvHandle(cbvsrvHeap->GetCPUDescriptorHandleForHeapStart(), 0, m_cbvsrvDescriptorSize);
		 D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		 srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		 srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		 srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		 srvDesc.Texture2D.MipLevels = 1;
		 device->CreateShaderResourceView(m_texture.Get(), &srvDesc, cbvsrvHandle);
		 cbvsrvHandle.Offset(m_cbvsrvDescriptorSize);

		 // Describe and create a SRV for the texture.
		 D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2 = {};
		 srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		 srvDesc2.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		 srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		 srvDesc2.Texture2D.MipLevels = 1;
		 device->CreateShaderResourceView(m_texture2.Get(), &srvDesc2, cbvsrvHandle);
		 cbvsrvHandle.Offset(m_cbvsrvDescriptorSize);
		 
		 // Describe and create a SRV for the texture.
		 D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc3 = {};
		 srvDesc3.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		 srvDesc3.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		 srvDesc3.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		 srvDesc3.Texture2D.MipLevels = 1;
		 device->CreateShaderResourceView(m_mat1.Get(), &srvDesc3, cbvsrvHandle);
		 cbvsrvHandle.Offset(m_cbvsrvDescriptorSize);

		 // Describe and create a SRV for the texture.
		 D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc4 = {};
		 srvDesc4.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		 srvDesc4.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		 srvDesc4.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		 srvDesc4.Texture2D.MipLevels = 1;
		 device->CreateShaderResourceView(m_mat2.Get(), &srvDesc4, cbvsrvHandle);
		 cbvsrvHandle.Offset(m_cbvsrvDescriptorSize);

		 // Describe and create a SRV for the texture.
		 D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc5 = {};
		 srvDesc5.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		 srvDesc5.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		 srvDesc5.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		 srvDesc5.Texture2D.MipLevels = 1;
		 device->CreateShaderResourceView(m_mat3.Get(), &srvDesc5, cbvsrvHandle);
		 cbvsrvHandle.Offset(m_cbvsrvDescriptorSize);

		 // Describe and create a SRV for the texture.
		 D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc6 = {};
		 srvDesc6.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		 srvDesc6.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		 srvDesc6.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		 srvDesc6.Texture2D.MipLevels = 1;
		 device->CreateShaderResourceView(m_mat4.Get(), &srvDesc6, cbvsrvHandle);
		 cbvsrvHandle.Offset(m_cbvsrvDescriptorSize);

		 //Initialize texture manager
		 text_mgr.Initialize(device.Get(), command_list.Get(), cbvsrvHeap.Get(), 6, 128);

		 text_mgr.CreateTexture();
		 
		 

		 testT = new Mesh(device.Get(), command_list.Get(), vertices, indices);

		 level.Initialize(4, 4, device.Get(), command_list.Get(), &camera);

		 // Root Constant Buffer
		 rootConstantBufferData.ambientLightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		 rootConstantBufferData.ambientLightStrength = 0.8f;

		 const UINT cbvBufferSize = static_cast<UINT>(sizeof(CB_PS_light) + (256 - sizeof(CB_PS_light) % 256));
		 const CD3DX12_HEAP_PROPERTIES rcb_heap_props(D3D12_HEAP_TYPE_UPLOAD);
		 const CD3DX12_RESOURCE_DESC rcb_resource_desc = CD3DX12_RESOURCE_DESC::Buffer(cbvBufferSize);

		 // Create Root CBV
		 hr = device->CreateCommittedResource(
			 &rcb_heap_props,
			 D3D12_HEAP_FLAG_NONE,
			 &rcb_resource_desc,
			 D3D12_RESOURCE_STATE_GENERIC_READ,
			 nullptr, __uuidof(ID3D12Resource), (void**)rootConstantBuffer.GetAddressOf()
		 );
		 COM_ERROR_IF_FAILED(hr, "Failed to create Root CBV.");

		 D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		 cbvDesc.BufferLocation = rootConstantBuffer->GetGPUVirtualAddress();
		 cbvDesc.SizeInBytes = cbvBufferSize;
		 CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(cbvsrvHeap->GetCPUDescriptorHandleForHeapStart(), 128, m_cbvsrvDescriptorSize);
		 device->CreateConstantBufferView(&cbvDesc, cbvHandle);

		 CD3DX12_RANGE readRange(0, 0);
		 UINT8* cbvDataBegin;
		 hr = rootConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&cbvDataBegin));
		 COM_ERROR_IF_FAILED(hr, "Failed to map root constant buffer.");

		 memcpy(cbvDataBegin, &rootConstantBufferData, sizeof(rootConstantBufferData));

		 rootConstantBuffer->Unmap(0, nullptr);

		 //Initialize ConstantBuffer
		 hr = cb_world.Initialize(device.Get(), command_list.Get(), cbvsrvHeap.Get(), 2, 129);
		 COM_ERROR_IF_FAILED(hr, "Failed to initialize ConstantBuffer.");

		 cb_world.UpdateConstantBuffer(0, cb_world_data);

		 //Needs constant buffer
		 if (!test_go.Initialize("Data\\Models\\female.obj" ,device.Get(), command_list.Get(), &cb_world))
			 exit(-1);

		 test_go.AdjustPosition(1.0f, 0.0f, 1.0f);

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
	 test_go.ReleaseCreationResources();
	 testT->ReleaseLoadingResources();
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
			 hr = command_list->Reset(command_allocator[m_frameIndex].Get(), pipeline_state_quad.Get());
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

		 CD3DX12_GPU_DESCRIPTOR_HANDLE cbvsrvHandle(cbvsrvHeap->GetGPUDescriptorHandleForHeapStart(), 0, m_cbvsrvDescriptorSize);
		 command_list->SetGraphicsRootDescriptorTable(0, cbvsrvHandle);
		 cbvsrvHandle.Offset(m_cbvsrvDescriptorSize);
		 cbvsrvHandle.Offset(m_cbvsrvDescriptorSize); //Good enough for now
		 command_list->SetGraphicsRootDescriptorTable(3, cbvsrvHandle);
		 cbvsrvHandle.Offset(m_cbvsrvDescriptorSize);
		 command_list->SetGraphicsRootConstantBufferView(2, rootConstantBuffer->GetGPUVirtualAddress());
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
		 
		 //tesselation test
		 //command_list->SetPipelineState(pipeline_state_quad.Get());
		 command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

		 cb_world.SetConstantBuffer(0);
		 
		 command_list->SetGraphicsRoot32BitConstant(4, tess, 0);
		 //testT->Render();

		 //Draw level
		 //command_list->SetPipelineState(pipeline_state.Get());
		 //command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		 level.RenderMap();

		 //Draw entities on map
		 command_list->SetPipelineState(pipeline_state.Get());
		 command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		 level.RenderEntities();

		 //text_mgr.SetTexture(0);
		 CD3DX12_GPU_DESCRIPTOR_HANDLE cbvsrvHandle2(cbvsrvHeap->GetGPUDescriptorHandleForHeapStart(), 1, m_cbvsrvDescriptorSize);
		 command_list->SetGraphicsRootDescriptorTable(0, cbvsrvHandle2);
		 cbvsrvHandle.Offset(m_cbvsrvDescriptorSize);

		 command_list->SetGraphicsRoot32BitConstant(4, tess, 0);
		 //Draw model
		 test_go.Render(camera.GetViewMatrix() * camera.GetProjectionMatrix());

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
