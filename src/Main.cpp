#include "Engine/Engine.h"
#include "Utility/Config.h"
//Console includes
/*#include <stdio.h>
#include <io.h>
#include <fcntl.h>*/

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	//Standard console
	/*AllocConsole();

	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
	FILE* hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, NULL, _IONBF, 1);
	*stdout = *hf_out;

	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
	FILE* hf_in = _fdopen(hCrt, "r");
	setvbuf(hf_in, NULL, _IONBF, 128);
	*stdin = *hf_in;*/

	Config::GetConfig();

	Engine engine;
	engine.Initialize(hInstance, "HelloWord", "MyWindowClass", 1360, 765);
	while (engine.ProcessMessages() == true)
	{
		engine.Run();
	}

	return 0;
}