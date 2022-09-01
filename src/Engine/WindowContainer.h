#pragma once
#include "RenderWindow.h"
#include "Input/KeyboardClass.h"
#include "Input/MouseClass.h"
#include "Graphics/Graphics.h"

class WindowContainer {

public:
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Destroy();
protected:
	WindowContainer();
	Graphics* gfx = nullptr;
	RenderWindow render_window;
	KeyboardClass keyboard;
	MouseClass mouse;
private:

};