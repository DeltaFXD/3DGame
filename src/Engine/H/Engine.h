#pragma once
#include "Engine/H/WindowContainer.h"
#include "Utility/H/Timer.h"

class Engine : WindowContainer{

public:
	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	bool ProcessMessages();
	void Update();
	void Render();
private:
	Timer timer;
};