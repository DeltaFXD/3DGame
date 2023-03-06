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

	static wrl::ComPtr<ID3DBlob> CreateShader(std::wstring shaderpath, const D3D_SHADER_MACRO* defines, std::string entryPoint, ShaderType type, UINT compileFlags)
	{
		HRESULT hr;
		std::string target = "";
		wrl::ComPtr<ID3DBlob> shader = nullptr;
		wrl::ComPtr<ID3DBlob> error;

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

		hr = D3DCompileFromFile(shaderpath.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), target.c_str(), compileFlags, 0, &shader, &error);
		if (FAILED(hr))
		{
			std::wstring errorMsg = L"Failed to load shader: ";
			errorMsg += StringConverter::StringToWide(target);
			errorMsg += L" path: ";
			errorMsg += shaderpath;
			errorMsg += L"\n Compile error:\n";
			errorMsg += StringConverter::StringToWide((char*)error->GetBufferPointer());
			ErrorLogger::Log(hr, errorMsg);

			//Failed to load shader exit program
			exit(-1);
		}

		return shader;
	}
};