#include "Render/H/RenderWindow.h"
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"DirectXTK12.lib")

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	
	RenderWindow rw;
	rw.Initialize(hInstance, "HelloWord", "MyWindowClass", 800, 600);
	while (rw.ProcessMessages() == true) {
		Sleep(50);
	}

	return 0;
}