#pragma once
#include "WindowContainer.h"
#include "Utility/Timer.h"

class Engine : WindowContainer{

public:
	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	bool ProcessMessages();
	void Run();
private:
	void Render();
	void Update();
	Timer timer;

	int frames = 0;
	int updates = 0;
	double delta = 0.0;
};