#include "Engine/H/Engine.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	timer.Start();
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
	float delta = timer.GetMilisecondsElapsed();
	timer.Restart();
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
		if (mouse.IsLeftDown())
		{
			if (e.GetType() == MouseEvent::EventType::RAW_MOVE)
			{
				gfx.camera.AdjustRotation((float)e.GetPosY() * 0.0001f * delta, (float)e.GetPosX() * 0.0001f * delta, 0.0f);
			}
		}
	}
	const float cameraSpeed = 0.02f;

	if (keyboard.KeyIsPressed('W'))
	{
		gfx.camera.AdjustPosition(gfx.camera.GetForwardVector() * cameraSpeed * delta);
	}
	if (keyboard.KeyIsPressed('S'))
	{
		gfx.camera.AdjustPosition(gfx.camera.GetBackwardVector() * cameraSpeed * delta);
	}
	if (keyboard.KeyIsPressed('A'))
	{
		gfx.camera.AdjustPosition(gfx.camera.GetLeftVector() * cameraSpeed * delta);
	}
	if (keyboard.KeyIsPressed('D'))
	{
		gfx.camera.AdjustPosition(gfx.camera.GetRightVector() * cameraSpeed * delta);
	}
	if (keyboard.KeyIsPressed(VK_SPACE))
	{
		gfx.camera.AdjustPosition(0.0f, cameraSpeed * delta, 0.0f);
	}
	if (keyboard.KeyIsPressed('X'))
	{
		gfx.camera.AdjustPosition(0.0f, -cameraSpeed * delta, 0.0f);
	}

	gfx.Update();
}

void Engine::Render()
{
	gfx.Render();
}
