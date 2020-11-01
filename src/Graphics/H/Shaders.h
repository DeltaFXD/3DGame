#pragma once
#include "Utility/H/ErrorLogger.h"
#pragma comment(lib, "D3DCompiler.lib")
#include <d3d12.h>
#include <wrl/client.h>
#include <d3dcompiler.h>

namespace wrl = Microsoft::WRL;

class VertexShader
{
public:
	bool Initialize(std::wstring shaderpath, UINT compileFlags);
	ID3DBlob* GeBuffer();
	ID3D12Resource* GetShader();
private:
	wrl::ComPtr<ID3DBlob> shader_buffer = nullptr;
	wrl::ComPtr<ID3D12Resource> shader = nullptr;
};

class PixelShader
{
public:
	bool Initialize(std::wstring shaderpath, UINT compileFlags);
	ID3DBlob* GeBuffer();
private:
	wrl::ComPtr<ID3DBlob> shader_buffer = nullptr;
};