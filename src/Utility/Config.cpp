#include "Config.h"

UINT Config::BUFFER_FRAME_COUNT = 2; // Minimum 2: front and back buffer
bool Config::v_sync = false;

UINT Config::GetBufferFrameCount()
{
	return BUFFER_FRAME_COUNT;
}

bool Config::IsVSyncOn()
{
	return v_sync;
}