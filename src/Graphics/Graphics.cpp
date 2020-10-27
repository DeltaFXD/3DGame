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
	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { command_list.Get() };
	command_queue.Get()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	HRESULT hr;
	hr = swapchain.Get()->Present(1, 0);
	if (FAILED(hr))
	{
		hr = device.Get()->GetDeviceRemovedReason();
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

	IDXGISwapChain1* temp_swap_chain;
	hr = factory.Get()->CreateSwapChainForHwnd(command_queue.Get(), hwnd, &scd, &scfd, NULL, &temp_swap_chain);

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
	
	// Next upgrade the IDXGISwapChain1 to a IDXGISwapChain3 interface and store it in a private member variable named m_swapChain.
	// This will allow us to use the newer functionality such as getting the current back buffer index.
	hr = temp_swap_chain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)swapchain.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to upgrade SwapChain.");
		exit(-1);
	}
	temp_swap_chain = nullptr;

	m_frameIndex = swapchain.Get()->GetCurrentBackBufferIndex();

	//Factory no longer needed
	factory.Get()->Release();

	// Describe and create a render target view (RTV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = Config::GetBufferFrameCount();
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = device.Get()->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)heap_desc.GetAddressOf());

	m_rtvDescriptorSize = device.Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create ID3D12DescriptorHeap");
		exit(-1);
	}
	//--------frame0
	hr = swapchain.Get()->GetBuffer(0, __uuidof(ID3D12Resource), (void**)render_target_view[0].GetAddressOf());

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to get back buffer.");
		exit(-1);
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(heap_desc.Get()->GetCPUDescriptorHandleForHeapStart());

	device.Get()->CreateRenderTargetView(render_target_view[0].Get(), nullptr, rtvHandle);
	rtvHandle.Offset(1, m_rtvDescriptorSize);
	//-----------frame 1
	hr = swapchain.Get()->GetBuffer(1, __uuidof(ID3D12Resource), (void**)render_target_view[1].GetAddressOf());

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to get back buffer.");
		exit(-1);
	}

	device.Get()->CreateRenderTargetView(render_target_view[1].Get(), nullptr, rtvHandle);
	rtvHandle.Offset(1, m_rtvDescriptorSize);
	//---------frame 1 end

	hr = device.Get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)command_allocator.GetAddressOf());

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create ID3D12CommandAllocator.");
		exit(-1);
	}

#ifdef _DEBUG //Debug mode
	hr = device.Get()->QueryInterface(__uuidof(ID3D12InfoQueue), (void**)info.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to get Info Queue");
	}
	info.Get()->PushEmptyStorageFilter();
#endif
	
	//Set viewport
	m_viewport.Width = static_cast<float>(width);
	m_viewport.Height = static_cast<float>(height);

	//Set scissor rect
	m_scissorRect.left = 0;
	m_scissorRect.top = 0;
	m_scissorRect.right = static_cast<LONG>(width);
	m_scissorRect.bottom = static_cast<LONG>(height);

	return true;
}

void Graphics::Load()
{
	// Create an empty root signature.
	D3D12_ROOT_SIGNATURE_DESC rsd;
	rsd.NumParameters = 0;
	rsd.pParameters = nullptr;
	rsd.NumStaticSamplers = 0;
	rsd.pStaticSamplers = nullptr;
	rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	wrl::ComPtr<ID3DBlob> signature;
	wrl::ComPtr<ID3DBlob> error;

	HRESULT hr;

	hr = D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to SerializeRootSignature.");
		exit(-1);
	}

	hr = device.Get()->CreateRootSignature(0, signature.Get()->GetBufferPointer(), signature.Get()->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)root_signature.GetAddressOf());

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create RootSignature.");
		exit(-1);
	}

	wrl::ComPtr<ID3DBlob> vertexShader;
	wrl::ComPtr<ID3DBlob> pixelShader;

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
	hr = D3DCompileFromFile(vertexPath.c_str(), nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create VertexShader.");
		exit(-1);
	}
	std::wstring pixelPath = shaderfolder + L"pixelshader.hlsl";
	hr = D3DCompileFromFile(pixelPath.c_str(), nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create PixelShader.");
		exit(-1);
	}

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }//,
		//{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = root_signature.Get();
	psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	hr = device.Get()->CreateGraphicsPipelineState(&psoDesc, __uuidof(ID3D12PipelineState), (void**)pipeline_state.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create PipelineState.");
		exit(-1);
	}

	hr = device.Get()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator.Get(), pipeline_state.Get(), __uuidof(ID3D12GraphicsCommandList), (void**)command_list.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create CommandList.");
		exit(-1);
	}

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	hr = command_list.Get()->Close();
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to close CommandList.");
		exit(-1);
	}

	Vertex triangle[] =
	{
		{ 0.0f, 0.5f, 0.0f },
		{ 0.5f, -0.5f, 0.0f },
		{ -0.5f, -0.5f, 0.0f }
	};

	const UINT vertexBufferSize = sizeof(triangle);

	// Note: using upload heaps to transfer static data like vert buffers is not 
	// recommended. Every time the GPU needs it, the upload heap will be marshalled 
	// over. Please read up on Default Heap usage. An upload heap is used here for 
	// code simplicity and because there are very few verts to actually transfer.
	hr = device.Get()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, __uuidof(ID3D12Resource), (void**)vertex_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create VertexBuffer.");
		exit(-1);
	}

	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	hr = vertex_buffer.Get()->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to map VertexBuffer.");
		exit(-1);
	}
	memcpy(pVertexDataBegin, triangle, sizeof(triangle));
	vertex_buffer.Get()->Unmap(0, nullptr);

	m_vertexBufferView.BufferLocation = vertex_buffer.Get()->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_vertexBufferView.SizeInBytes = vertexBufferSize;

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	hr = device.Get()->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)m_fence.GetAddressOf());
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
	hr = command_queue.Get()->Signal(m_fence.Get(), fence);
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

	m_frameIndex = swapchain.Get()->GetCurrentBackBufferIndex();
 }

 void Graphics::PopulateCommandList()
 {
	 HRESULT hr;
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	hr = command_allocator.Get()->Reset();
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to Reset CommandAllocator.");
		exit(-1);
	}

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before re-recording.
	hr = command_list.Get()->Reset(command_allocator.Get(), pipeline_state.Get());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to Reset CommandList.");
		exit(-1);
	}

	// Set necessary state.
	command_list.Get()->SetGraphicsRootSignature(root_signature.Get());
	command_list.Get()->RSSetViewports(1, &m_viewport);
	command_list.Get()->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	command_list.Get()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(render_target_view[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(heap_desc.Get()->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	command_list.Get()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.5f, 0.0f, 0.5f, 1.0f };
	command_list.Get()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	command_list.Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	command_list.Get()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	command_list.Get()->DrawInstanced(3, 1, 0, 0);

	// Indicate that the back buffer will now be used to present.
	command_list.Get()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(render_target_view[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	hr = command_list.Get()->Close();
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to close CommandList.");
		exit(-1);
	}
 }
