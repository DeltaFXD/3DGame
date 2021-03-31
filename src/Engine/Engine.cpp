#include "Engine.h"

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
	timer.Set();
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
		if (mouse.IsLeftDown())
		{
			if (e.GetType() == MouseEvent::EventType::RAW_MOVE)
			{
				gfx.camera.AdjustRotation((float)e.GetPosY() * 0.0001f, (float)e.GetPosX() * 0.0001f, 0.0f);
			}
		}
	}
	const float cameraSpeed = 0.03f;

	
	if (keyboard.KeyIsPressed(VK_RIGHT))
	{
		gfx.test_go.AdjustRotation(0.0f, cameraSpeed, 0.0f);
	}

	if (keyboard.KeyIsPressed(VK_UP))
	{
		gfx.test_go.AdjustRotation(cameraSpeed, 0.0f, 0.0f);
	}

	if (keyboard.KeyIsPressed(VK_DOWN))
	{
		gfx.test_go.AdjustRotation(0.0f, 0.0f, cameraSpeed);
	}

	if (keyboard.KeyIsPressed('W'))
	{
		gfx.camera.AdjustPosition(0.0f , 0.0f, cameraSpeed);
	}
	if (keyboard.KeyIsPressed('S'))
	{
		gfx.camera.AdjustPosition(0.0f , 0.0f, -cameraSpeed);
	}
	if (keyboard.KeyIsPressed('A'))
	{
		gfx.camera.AdjustPosition(-cameraSpeed, 0.0f, 0.0f);
	}
	if (keyboard.KeyIsPressed('D'))
	{
		gfx.camera.AdjustPosition(cameraSpeed, 0.0f, 0.0f);
	}
	if (keyboard.KeyIsPressed(VK_SPACE))
	{
		gfx.camera.AdjustPosition(0.0f, cameraSpeed, 0.0f);
	}
	if (keyboard.KeyIsPressed('X'))
	{
		gfx.camera.AdjustPosition(0.0f, -cameraSpeed, 0.0f);
	}

	gfx.Update();
}

void Engine::Render()
{
	gfx.Render();
}

void Engine::Run()
{
	delta += timer.GetDelta();
	while (delta >= 1)
	{
		Update();
		updates++;
		delta--;
	}
	Render();
	frames++;
	if (timer.IsSecondPassed())
	{
		static std::string status = "";
		status = "FPS: " + std::to_string(frames) + " , UPS: " + std::to_string(updates) + " , X: " + std::to_string(gfx.camera.GetPositionFloat3().z) + " , Y: " + std::to_string(gfx.camera.GetPositionFloat3().x) + "\n";
		OutputDebugStringA(status.c_str());
		frames = 0;
		updates = 0;
	}
}