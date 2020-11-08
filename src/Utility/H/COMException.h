#pragma once
#include <comdef.h>
#include "StringConverter.h"

#define COM_ERROR_IF_FAILED(hr, msg) if (FAILED(hr)) throw COMException(hr, msg, __FILE__, __FUNCTION__, __LINE__);

class COMException
{
public:
	COMException(HRESULT hr, const std::string& msg, const std::string& file, const std::string& function, int line)
	{
		_com_error error(hr);
		m_msg = L"Msg: " + StringConverter::StringToWide(std::string(msg)) + L"\n";
		m_msg += error.ErrorMessage();
		m_msg += L"\nFile: " + StringConverter::StringToWide(file);
		m_msg += L"\nFunction: " + StringConverter::StringToWide(function);
		m_msg += L"\nLine: " + StringConverter::StringToWide(std::to_string(line));
	}

	const wchar_t* what() const
	{
		return m_msg.c_str();
	}
private:
	std::wstring m_msg;
};