#include "WindowContainer.h"
#include <hidusage.h>
#include <memory>

WindowContainer::WindowContainer()
{
	static bool raw_input_initialized = false;
	if (raw_input_initialized == false)
	{
		RAWINPUTDEVICE rid;
		//https://docs.microsoft.com/en-us/windows-hardware/drivers/hid/hid-usages#usage-page
		rid.usUsagePage = HID_USAGE_PAGE_GENERIC; //Generic Desktop Controls
		rid.usUsage = HID_USAGE_GENERIC_MOUSE; //Mouse
		rid.dwFlags = 0;
		rid.hwndTarget = NULL;

		if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
		{
			ErrorLogger::Log(GetLastError(), "Raw Input Device Registration failed.");
			exit(-1);
		}
		raw_input_initialized = true;
	}
}

LRESULT WindowContainer::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	//Keyboard Messages
	//https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-char
	//lparam bit:30 -> previous key state
	case WM_KEYDOWN:
	{
		unsigned char c = static_cast<unsigned char>(wParam);
		if (keyboard.IsKeyAutoRepeat())
		{
			keyboard.OnKeyPressed(c);
		}
		else
		{
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
	//Mouse Messages
	//https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-mousemove
	//lParam
	//The low - order word specifies the x - coordinate of the cursor.The coordinate is relative to the upper - left corner of the client area.
	//The high - order word specifies the y - coordinate of the cursor.The coordinate is relative to the upper - left corner of the client area.
	case WM_MOUSEMOVE:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		mouse.OnMouseMove(x, y);
		return 0;
	}
	//Buttons down
	case WM_LBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		mouse.OnLeftPressed(x, y);
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		mouse.OnRightPressed(x, y);
		return 0;
	}
	case WM_MBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		mouse.OnMiddlePressed(x, y);
		return 0;
	}
	//Buttons up
	case WM_LBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		mouse.OnLeftReleased(x, y);
		return 0;
	}
	case WM_RBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		mouse.OnRightReleased(x, y);
		return 0;
	}
	case WM_MBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		mouse.OnMiddleReleased(x, y);
		return 0;
	}
	//Wheel up/down
	case WM_MOUSEWHEEL:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
		{
			mouse.OnWheelUp(x, y);
		}
		else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
		{
			mouse.OnWheelDown(x, y);
		}
		break;
	}
	//Raw mouse input
	//https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-input
	case WM_INPUT:
	{
		UINT dataSize;

		GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));

		if (dataSize > 0)
		{
			std::unique_ptr<BYTE[]> rawdata = std::make_unique<BYTE[]>(dataSize);
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawdata.get(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
			{
				RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.get());
				if (raw->header.dwType == RIM_TYPEMOUSE)
				{
					mouse.OnMouseMoveRaw(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
				}
			}
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}