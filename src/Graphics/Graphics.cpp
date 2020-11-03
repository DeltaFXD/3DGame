#include "Graphics/H/Graphics.h"
#include "Utility/H/Config.h"

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
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

	if (!InitializeDirectX(hwnd, width, height))
	{
		return false;
	}

	Load(width, height);

	return true;
}

void Graphics::Render()
{
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { command_list.Get() };
	command_queue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	HRESULT hr;
	hr = swapchain->Present(1, 0);
	if (FAILED(hr))
	{
		hr = device->GetDeviceRemovedReason();
		ErrorLogger::Log(hr, "Failed to present frame.");
		exit(-1);
	}

	WaitForPreviousFrame();
}

bool Graphics::InitializeDirectX(HWND hwnd, int width, int height)
{
#pragma region D3Debug
	if (IsDebuggerPresent() == TRUE)
	{
#ifdef _DEBUG //Debug mode
		wrl::ComPtr<ID3D12Debug> debug_controller;
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

#ifdef _DEBUG //Debug mode
	hr = device->QueryInterface(__uuidof(ID3D12InfoQueue), (void**)info.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to get Info Queue");
	}
	info->PushEmptyStorageFilter();
	D3D12_INFO_QUEUE_FILTER_DESC filterDesc;
	D3D12_MESSAGE_SEVERITY msList[] = { D3D12_MESSAGE_SEVERITY_INFO };
	ZeroMemory(&filterDesc, sizeof(filterDesc));
	filterDesc.NumCategories = 0;
	filterDesc.pCategoryList = nullptr;
	filterDesc.NumSeverities = 1;
	filterDesc.pSeverityList = msList;
	D3D12_INFO_QUEUE_FILTER filter;
	ZeroMemory(&filter, sizeof(filter));
	filter.DenyList = filterDesc;
	hr = info->AddStorageFilterEntries(&filter);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to apply InfoQueue filter.");
	}
#endif

	//Command queue describer
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	hr = device->CreateCommandQueue(&queueDesc, __uuidof(ID3D12CommandQueue), (void**)command_queue.GetAddressOf());

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

	IDXGISwapChain1* temp_swap_chain;
	hr = factory->CreateSwapChainForHwnd(command_queue.Get(), hwnd, &scd, &scfd, NULL, &temp_swap_chain);

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
	
	// Next upgrade the IDXGISwapChain1 to a IDXGISwapChain3 interface and store it.
	// This will allow us to use the newer functionality such as getting the current back buffer index.
	hr = temp_swap_chain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)swapchain.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to upgrade SwapChain.");
		exit(-1);
	}
	temp_swap_chain = nullptr;

	m_frameIndex = swapchain->GetCurrentBackBufferIndex();

	//Factory no longer needed
	factory->Release();

	//Create Descriptor Heaps
	CreateDescriptorHeaps();

	m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//Create frame resources
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	//--------frame0
	hr = swapchain->GetBuffer(0, __uuidof(ID3D12Resource), (void**)render_target_view[0].GetAddressOf());

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to get back buffer.");
		exit(-1);
	}

	device->CreateRenderTargetView(render_target_view[0].Get(), nullptr, rtvHandle);
	rtvHandle.Offset(1, m_rtvDescriptorSize);
	//-----------frame 1
	hr = swapchain->GetBuffer(1, __uuidof(ID3D12Resource), (void**)render_target_view[1].GetAddressOf());

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to get back buffer.");
		exit(-1);
	}

	device->CreateRenderTargetView(render_target_view[1].Get(), nullptr, rtvHandle);
	rtvHandle.Offset(1, m_rtvDescriptorSize);
	//---------frame 1 end

	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)command_allocator.GetAddressOf());

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create ID3D12CommandAllocator.");
		exit(-1);
	}

	graphicsMemory = std::make_unique<DirectX::GraphicsMemory>(device.Get());

	//spriteHeap = std::make_unique<DirectX::DescriptorHeap>(textHeap.Get());

	DirectX::ResourceUploadBatch resourceUpload(device.Get());

	resourceUpload.Begin();

	hr = DirectX::CreateWICTextureFromFile(device.Get(), resourceUpload, L"Data\\Textures\\sample.png", m_texture.GetAddressOf(), false);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create wic texture from file.");
		exit(-1);
	}
	//spriteFont = std::make_unique<DirectX::SpriteFont>(device.Get(), resourceUpload, L"Data\\Fonts\\comic_sans_ms_16.spritefont", textHeap->GetCPUDescriptorHandleForHeapStart(), textHeap->GetGPUDescriptorHandleForHeapStart());

	//DirectX::RenderTargetState rtState(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D32_FLOAT);

	//DirectX::SpriteBatchPipelineStateDescription pd(rtState);
	//spriteBatch = std::make_unique<DirectX::SpriteBatch>(device.Get(), resourceUpload, pd);

	auto uploadResourcesFinished = resourceUpload.End(command_queue.Get());

	uploadResourcesFinished.wait();
	
	//spriteBatch->SetViewport(m_viewport);

	return true;
}

void Graphics::CreateDescriptorHeaps()
{
	HRESULT hr;
	// Describe and create a render target view (RTV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = Config::GetBufferFrameCount();
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = device->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)rtvHeap.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create RTV Heap");
		exit(-1);
	}

	// Describe and create a depth stencil view (DSV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = device->CreateDescriptorHeap(&dsvHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)dsvHeap.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create DSV Heap");
		exit(-1);
	}

	// Describe and create a shader resource view (SRV) heap for the texture.
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	hr = device->CreateDescriptorHeap(&srvHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)srvHeap.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create SRV Heap");
		exit(-1);
	}

	// Describe and create a sprite font descriptor heap.
	/*D3D12_DESCRIPTOR_HEAP_DESC sfHeapDesc = {};
	sfHeapDesc.NumDescriptors = 1;
	sfHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	sfHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	hr = device->CreateDescriptorHeap(&sfHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)textHeap.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create sprite font Heap");
		exit(-1);
	}*/
}

void Graphics::Load(int width, int height)
{
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

	CD3DX12_DESCRIPTOR_RANGE1 ranges[1] = {};
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

	CD3DX12_ROOT_PARAMETER1 rootParameters[1] = {};
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

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

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to SerializeRootSignature.");
		exit(-1);
	}

	hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)root_signature.GetAddressOf());

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create RootSignature.");
		exit(-1);
	}

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
			shaderfolder = L"";
	#else	//x86 (Win32)
			shaderfolder = L"";
	#endif //Release Mode
	#else
	#ifdef _WIN64 //x64
			shaderfolder = L"";
	#else	//x86 (Win32)
			shaderfolder = L"";
	#endif
#endif 
	}

	std::wstring vertexPath = shaderfolder + L"vertexshader.hlsl";
	if (!vertex_shader.Initialize(vertexPath, compileFlags))
	{
		exit(-1);
	}
	std::wstring pixelPath = shaderfolder + L"pixelshader.hlsl";
	if (!pixel_shader.Initialize(pixelPath, compileFlags))
	{
		exit(-1);
	}

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	//Create Rasterizer State
	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));

	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID; //SOLID / WIREFRAME
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;

	//DepthStencil
	D3D12_DEPTH_STENCIL_DESC depth_stencilDesc;
	ZeroMemory(&depth_stencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
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
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create PipelineState.");
		exit(-1);
	}

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator.Get(), pipeline_state.Get(), __uuidof(ID3D12GraphicsCommandList), (void**)command_list.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create CommandList.");
		exit(-1);
	}

	//------------triangle
	Vertex square[] =
	{
		Vertex(-0.5f, -0.5f, 1.0f, 0.0f, 1.0f), //Bottom Left	-	[0]
		Vertex(-0.5f,  0.5f, 1.0f, 0.0f, 0.0f), //Top Left		-	[1]
		Vertex( 0.5f,  0.5f, 1.0f, 1.0f, 0.0f), //Top Right		-	[2]
		Vertex( 0.5f, -0.5f, 1.0f, 1.0f, 1.0f)  //Bottom Right	-	[3]
	};

	DWORD indices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	const UINT vertexBufferSize = sizeof(square);
	const UINT indexBufferSize = sizeof(indices);

	// Note: using upload heaps to transfer static data like vert buffers is not 
	// recommended. Every time the GPU needs it, the upload heap will be marshalled 
	// over. Please read up on Default Heap usage. An upload heap is used here for 
	// code simplicity and because there are very few verts to actually transfer.
	hr = device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(ID3D12Resource), (void**)vertex_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create VertexBuffer.");
		exit(-1);
	}

	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	hr = vertex_buffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to map VertexBuffer.");
		exit(-1);
	}
	memcpy(pVertexDataBegin, square, sizeof(square));
	vertex_buffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	m_vertexBufferView.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
	//----------------

	hr = device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(ID3D12Resource), (void**)index_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create IndexBuffer.");
		exit(-1);
	}
	//Copy index data to buffer.
	UINT* pIndexDataBegin;
	CD3DX12_RANGE readRange2(0, 0);
	hr = index_buffer->Map(0, &readRange2, reinterpret_cast<void**>(&pIndexDataBegin));
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to map IndexBuffer.");
		exit(-1);
	}
	memcpy(pIndexDataBegin, indices, sizeof(indices));
	index_buffer->Unmap(0, nullptr);

	//Initialize the index buffer view
	m_indexBufferView.BufferLocation = index_buffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = indexBufferSize;
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// Describe and create a SRV for the texture.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	device->CreateShaderResourceView(m_texture.Get(), &srvDesc, srvHeap->GetCPUDescriptorHandleForHeapStart());
	// Close the command list and execute it to begin the initial GPU setup.
	hr = command_list->Close();
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to close command list to begin GPU setup.");
	}

	//Create depth stencil view
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D24_UNORM_S8_UINT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		__uuidof(ID3D12Resource),
		(void**)depth_stencil.GetAddressOf()
	);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create Stencil Buffer");
		exit(-1);
	}

	device->CreateDepthStencilView(depth_stencil.Get(), &depthStencilDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)m_fence.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create synchronization object.");
		exit(-1);
	}
	m_fenceValue = 1;

	// Create an event handle to use for frame synchronization.
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		ErrorLogger::Log(GetLastError(), "Failed to create event handle.");
		exit(-1);
	}

	// Wait for the command list to execute; we are reusing the same command 
	// list in our main loop but for now, we just want to wait for setup to 
	// complete before continuing.
	WaitForPreviousFrame();
 }

 void Graphics::WaitForPreviousFrame()
 {
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. More advanced samples 
	// illustrate how to use fences for efficient resource usage.

	// Signal and increment the fence value.
	const UINT64 fence = m_fenceValue;
	HRESULT hr;
	hr = command_queue->Signal(m_fence.Get(), fence);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to Signal.");
		exit(-1);
	}
	m_fenceValue++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		hr = m_fence->SetEventOnCompletion(fence, m_fenceEvent);
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, "Failed to Wait.");
			exit(-1);
		}
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = swapchain->GetCurrentBackBufferIndex();
 }

 void Graphics::PopulateCommandList()
 {
	 HRESULT hr;
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	hr = command_allocator->Reset();
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to Reset CommandAllocator.");
		exit(-1);
	}

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before re-recording.
	hr = command_list->Reset(command_allocator.Get(), pipeline_state.Get());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to Reset CommandList.");
		exit(-1);
	}

	// Set necessary state.
	command_list->SetGraphicsRootSignature(root_signature.Get());
	ID3D12DescriptorHeap* ppHeaps[] = { srvHeap.Get() };
	command_list->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	command_list->SetGraphicsRootDescriptorTable(0, srvHeap->GetGPUDescriptorHandleForHeapStart());
	command_list->RSSetViewports(1, &m_viewport);
	command_list->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(render_target_view[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());
	command_list->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	command_list->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	command_list->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//Draw triangle
	command_list->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	command_list->IASetIndexBuffer(&m_indexBufferView);
	command_list->DrawIndexedInstanced(6, 1, 0, 0, 0);

	//Draw text
	//TODO: implement better text rendering https://www.braynzarsoft.net/viewtutorial/q16390-11-drawing-text-in-directx-12
	/*ID3D12DescriptorHeap* heaps[] = { spriteHeap->Heap() };
	command_list->SetDescriptorHeaps(_countof(heaps), heaps);
	spriteBatch->Begin(command_list.Get());

	spriteFont->DrawString(spriteBatch.get(), L"Hello World!", DirectX::XMFLOAT2(0, 0), DirectX::Colors::White, 0.0f, DirectX::XMFLOAT2(0, 0), DirectX::XMFLOAT2(1, 1));

	spriteBatch->End();*/

	// Indicate that the back buffer will now be used to present.
	command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(render_target_view[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	hr = command_list->Close();
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to close CommandList.");
		exit(-1);
	}
 }
