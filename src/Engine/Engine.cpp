#include "Engine/H/Engine.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	return this->render_window.Initialize(this, hInstance, window_title, window_class, width, height);
}

bool Engine::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void Engine::Update()
{
	while (!keyboard.CharBufferIsEmpty())
	{
		unsigned char c = keyboard.ReadChar();
		std::string outmsg = "Char: ";
		outmsg += c;
		outmsg += "\n";
		OutputDebugStringA(outmsg.c_str());
	}
	while (!keyboard.KeyBufferIsEmpty())
	{
		KeyboardEvent e = keyboard.ReadKey();
		unsigned char c = e.GetKeyCode();
		std::string outmsg = "";
		if (e.IsPressed())
		{
			outmsg += "Key press: ";
		}
		if (e.IsReleased())
		{
			outmsg += "Key release: ";
		}
		outmsg += c;
		outmsg += "\n";
		OutputDebugStringA(outmsg.c_str());
	}
}