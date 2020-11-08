#include "Engine/Engine.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	Engine engine;
	engine.Initialize(hInstance, "HelloWord", "MyWindowClass", 1360, 765);
	while (engine.ProcessMessages() == true)
	{
		engine.Update();
		engine.Render();
	}

	return 0;
}