#include "Engine/H/Engine.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	if (!this->render_window.Initialize(this, hInstance, window_title, window_class, width, height))
	{
		return false;
	}

	if (gfx.Initialize(this->render_window.GetHWND(), width, height))
	{
		return false;
	}

	return true;
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
		/*std::string outmsg = "Char: ";
		outmsg += c;
		outmsg += "\n";
		OutputDebugStringA(outmsg.c_str());*/
	}
	while (!keyboard.KeyBufferIsEmpty())
	{
		KeyboardEvent e = keyboard.ReadKey();
		unsigned char c = e.GetKeyCode();
		/*std::string outmsg = "";
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
		OutputDebugStringA(outmsg.c_str());*/
	}
	while (!mouse.EventBufferIsEmpty())
	{
		MouseEvent e = mouse.ReadEvent();
		/*std::string outmsg = "X: ";
		outmsg += std::to_string(e.GetPosX());
		outmsg += ", Y: ";
		outmsg += std::to_string(e.GetPosY());
		outmsg += "\n";
		OutputDebugStringA(outmsg.c_str());*/
		/*if (e.GetType() == MouseEvent::EventType::WheelUp)
		{
			OutputDebugStringA("Mouse Wheel Up\n");
		}
		if (e.GetType() == MouseEvent::EventType::WheelDown)
		{
			OutputDebugStringA("Mouse Wheel Down\n");
		}*/
		if (e.GetType() == MouseEvent::EventType::RAW_MOVE)
		{
			/*std::string outmsg = "X: ";
			outmsg += std::to_string(e.GetPosX());
			outmsg += ", Y: ";
			outmsg += std::to_string(e.GetPosY());
			outmsg += "\n";
			OutputDebugStringA(outmsg.c_str());*/
		}
	}
}

void Engine::Render()
{
	gfx.Render();
}
