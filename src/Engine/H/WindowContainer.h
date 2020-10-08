#pragma once
#include "Render/H/RenderWindow.h"
#include "Input/H/KeyboardClass.h"

class WindowContainer {

public:
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	RenderWindow render_window;
	KeyboardClass keyboard;
private:

};