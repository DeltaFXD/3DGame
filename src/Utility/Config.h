#pragma once
#include <Windows.h>

class Config
{
public:
	static UINT GetBufferFrameCount();
	static bool IsVSyncOn();
private:
	static UINT BUFFER_FRAME_COUNT;
	static bool v_sync;
};