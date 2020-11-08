#include "Config.h"

UINT Config::BUFFER_FRAME_COUNT = 2; // Minimum 2: front and back buffer

UINT Config::GetBufferFrameCount()
{
	return BUFFER_FRAME_COUNT;
}