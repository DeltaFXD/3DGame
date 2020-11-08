#include "Shaders.h"

bool VertexShader::Initialize(std::wstring shaderpath, UINT compileFlags)
{
	HRESULT hr;
	hr = D3DCompileFromFile(shaderpath.c_str(), nullptr, nullptr, "main", "vs_5_0", compileFlags, 0, &shader_buffer, nullptr);
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	return true;
}

ID3DBlob* VertexShader::GeBuffer()
{
	return shader_buffer.Get();
}

ID3D12Resource* VertexShader::GetShader()
{
	return shader.Get();
}

bool PixelShader::Initialize(std::wstring shaderpath, UINT compileFlags)
{
	HRESULT hr;
	hr = D3DCompileFromFile(shaderpath.c_str(), nullptr, nullptr, "main", "ps_5_0", compileFlags, 0, &shader_buffer, nullptr);
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	return true;
}

ID3DBlob* PixelShader::GeBuffer()
{
	return shader_buffer.Get();
}
