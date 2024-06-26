#pragma once
#include "Utility/ErrorLogger.h"
#include <d3d12.h>
#include <wrl/client.h>
#include <dxgi.h>
#include <dxgi1_5.h>

namespace wrl = Microsoft::WRL;

class AdapterReader
{
public:
	static void GetHardwareAdapter(IDXGIFactory5* pFactory, IDXGIAdapter1** pAdapter);

private:
	static IDXGIAdapter1* adapter;
};