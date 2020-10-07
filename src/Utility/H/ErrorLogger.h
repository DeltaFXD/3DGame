#pragma once
#include "Utility/H/StringConverter.h"
#include "Windows.h"

class ErrorLogger {

public:
	static void Log(std::string message);
	static void Log(HRESULT hr, std::string message);
};