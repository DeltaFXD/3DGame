#include "Engine/H/WindowContainer.h"

LRESULT WindowContainer::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
	{
		unsigned char c = static_cast<unsigned char>(wParam);
		if (keyboard.IsKeyAutoRepeat())
		{
			keyboard.OnKeyPressed(c);
		}
		else
		{
			//https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-char bit:30 -> previous key state
			const bool wasPressed = lParam & 0x40000000;
			if (!wasPressed)
			{
				keyboard.OnKeyPressed(c);
			}
		}
		return 0;
	}
	case WM_KEYUP:
	{
		unsigned char c = static_cast<unsigned char>(wParam);
		keyboard.OnKeyReleased(c);
		return 0;
	}
	case WM_CHAR:
	{
		unsigned char c = static_cast<unsigned char>(wParam);
		if (keyboard.IsCharsAutoRepeat())
		{
			keyboard.OnChar(c);
		}
		else
		{
			const bool wasPressed = lParam & 0x40000000;
			if (!wasPressed)
			{
				keyboard.OnChar(c);
			}
		}
		return 0;
	}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}