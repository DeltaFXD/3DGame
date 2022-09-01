#include "Shaders.h"

bool Shader::Initialize(std::wstring shaderpath, const D3D_SHADER_MACRO* defines, std::string entryPoint, ShaderType type, UINT compileFlags)
{
	HRESULT hr;
	std::string target = "";
	
	switch (type)
	{
	case ShaderType::VS:
	{
		target = "vs_5_1";
		break;
	}
	case ShaderType::HS:
	{
		target = "hs_5_1";
		break;
	}
	case ShaderType::DS:
	{
		target = "ds_5_1";
		break;
	}
	case ShaderType::PS:
	{
		target = "ps_5_1";
		break;
	}
	}
	
	hr = D3DCompileFromFile(shaderpath.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), target.c_str(), compileFlags, 0, &shader_buffer, nullptr);
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += StringConverter::StringToWide(target);
		errorMsg += L" path: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	return true;
}

ID3DBlob* Shader::GeBuffer()
{
	return shader_buffer.Get();
}