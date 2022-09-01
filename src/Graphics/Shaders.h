#pragma once
#include "Utility/ErrorLogger.h"
#include <d3d12.h>
#include <wrl/client.h>
#include <d3dcompiler.h>

namespace wrl = Microsoft::WRL;

enum class ShaderType
{
	VS,
	HS,
	DS,
	PS
};

class Shader
{
public:
	bool Initialize(std::wstring shaderpath, const D3D_SHADER_MACRO* defines, std::string entryPoint, ShaderType type, UINT compileFlags);
	ID3DBlob* GeBuffer();
private:
	wrl::ComPtr<ID3DBlob> shader_buffer = nullptr;
};