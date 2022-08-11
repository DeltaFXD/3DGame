#include "Engine.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	if (!this->render_window.Initialize(this, hInstance, window_title, window_class, width, height))
	{
		return false;
	}

	if (Graphics::Initialize(this->render_window.GetHWND(), width, height))
	{
		gfx = Graphics::Get();
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
				gfx->camera.AdjustRotation((float)e.GetPosY() * 0.0001f, (float)e.GetPosX() * 0.0001f, 0.0f);
			}
		}
		if (e.GetType() == MouseEvent::EventType::WheelUp)
		{
			fov--;
			gfx->camera.SetProjectionValues(fov, static_cast<float>(gfx->GetWidth()) / static_cast<float>(gfx->GetHeight()), 0.1f, 1000.0f);
		}
		else if (e.GetType() == MouseEvent::EventType::WheelDown)
		{
			fov++;
			gfx->camera.SetProjectionValues(fov, static_cast<float>(gfx->GetWidth()) / static_cast<float>(gfx->GetHeight()), 0.1f, 1000.0f);
		}
	}
	static float cameraSpeed = 0.03f;
	if (keyboard.KeyIsPressed(VK_SHIFT))
	{
		cameraSpeed = 0.5f;
	}
	else
	{
		cameraSpeed = 0.03f;
	}

	if (keyboard.KeyIsPressed('Z'))
	{
		gfx->ChangeFillMode(keyboard.KeyIsToggled('Z'));
	}
	
	if (keyboard.KeyIsPressed(VK_RIGHT))
	{
		gfx->test_go.AdjustPosition(cameraSpeed, 0.0f, 0.0f);
	}

	if (keyboard.KeyIsPressed(VK_LEFT))
	{
		gfx->test_go.AdjustPosition(-cameraSpeed, 0.0f, 0.0f);
	}

	if (keyboard.KeyIsPressed(VK_UP))
	{
		gfx->test_go.AdjustPosition(0.0f, 0.0f, cameraSpeed);
	}

	if (keyboard.KeyIsPressed('O'))
	{
		static bool prev = false;

		if (keyboard.KeyIsToggled('O') != prev) {
			gfx->tess++;
			prev = !prev;
		}
	}
	if (keyboard.KeyIsPressed('L'))
	{
		static bool prev = false;

		if (keyboard.KeyIsToggled('L') != prev) {
			gfx->tess--;
			prev = !prev;
		}
	}

	if (keyboard.KeyIsPressed(VK_DOWN))
	{
		gfx->test_go.AdjustPosition(0.0f, 0.0f, -cameraSpeed);
	}
	gfx->test_go.SetPosition(gfx->level.GetHeight(gfx->test_go.GetPositionFloat3().x, gfx->test_go.GetPositionFloat3().z) + 0.01f);

	if (keyboard.KeyIsPressed('W'))
	{
		gfx->camera.AdjustPosition(0.0f , 0.0f, cameraSpeed);
	}
	if (keyboard.KeyIsPressed('S'))
	{
		gfx->camera.AdjustPosition(0.0f , 0.0f, -cameraSpeed);
	}
	if (keyboard.KeyIsPressed('A'))
	{
		gfx->camera.AdjustPosition(-cameraSpeed, 0.0f, 0.0f);
	}
	if (keyboard.KeyIsPressed('D'))
	{
		gfx->camera.AdjustPosition(cameraSpeed, 0.0f, 0.0f);
	}
	if (keyboard.KeyIsPressed(VK_SPACE))
	{
		gfx->camera.AdjustPosition(0.0f, cameraSpeed, 0.0f);
	}
	if (keyboard.KeyIsPressed('X'))
	{
		gfx->camera.AdjustPosition(0.0f, -cameraSpeed, 0.0f);
	}
	if (keyboard.KeyIsPressed(VK_CONTROL))
	{
		gfx->camera.SetPosition(gfx->level.GetHeight(gfx->camera.GetPositionFloat3().x, gfx->camera.GetPositionFloat3().z) + 1.0f);
	}

	gfx->Update();
}

void Engine::Render()
{
	gfx->Render();
}

void Engine::Run()
{
	if (gfx == nullptr) return;

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
		status = "FPS: " + std::to_string(frames) + 
			" , UPS: " + std::to_string(updates) + 
			" , FoV: " + std::to_string(fov) +
			" , X: " + std::to_string(gfx->camera.GetPositionFloat3().z) + 
			" , Y: " + std::to_string(gfx->camera.GetPositionFloat3().x) + 
			" , Z: " + std::to_string(gfx->camera.GetPositionFloat3().y) + "\n";
		OutputDebugStringA(status.c_str());
		frames = 0;
		updates = 0;
	}
}