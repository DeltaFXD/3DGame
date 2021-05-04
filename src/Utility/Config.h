#pragma once
#include <Windows.h>

class Config
{
public:
	static bool IsVSyncOn();
private:
	static bool v_sync;
};