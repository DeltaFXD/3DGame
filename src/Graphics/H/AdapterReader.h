#pragma once
#include "Utility/H/ErrorLogger.h"
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "DirectXTK12.lib")
#pragma comment(lib, "DXGI.lib")
#include <wrl/client.h>
#include <dxgi.h>
#include <dxgi1_4.h>

namespace wrl = Microsoft::WRL;

class AdapterReader
{
public:
	static void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** pAdapter);

private:
	static IDXGIAdapter1* adapter;
};