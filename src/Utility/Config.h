#pragma once
#include <Windows.h>

class Config
{
public:
	static UINT GetBufferFrameCount();
private:
	static UINT BUFFER_FRAME_COUNT;
};