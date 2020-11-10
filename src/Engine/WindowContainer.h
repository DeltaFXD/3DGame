#pragma once
#include "Render/RenderWindow.h"
#include "Input/KeyboardClass.h"
#include "Input/MouseClass.h"
#include "Graphics/Graphics.h"

class WindowContainer {

public:
	LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Destroy();
protected:
	WindowContainer();
	RenderWindow render_window;
	KeyboardClass keyboard;
	MouseClass mouse;
	Graphics gfx;
private:

};