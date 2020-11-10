#include "Engine/WindowContainer.h"

bool RenderWindow::Initialize(WindowContainer * pWindowContainer, HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	this->hInstance = hInstance;
	this->width = width;
	this->height = height;
	this->window_title = window_title;
	this->window_title_wide = StringConverter::StringToWide(this->window_title);
	this->window_class = window_class;
	this->window_class_wide = StringConverter::StringToWide(this->window_class);

	this->RegisterWindowClass();

	int centerScreenX = GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2;
	int centerScreenY = GetSystemMetrics(SM_CYSCREEN) / 2 - height / 2;

	RECT wr; //Window Rectangle
	wr.left = centerScreenX;
	wr.top = centerScreenY;
	wr.right = wr.left + width;
	wr.bottom = wr.top + height;
	AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

	this->handle = CreateWindowEx(0, //Extended Windows style - 0 default
		this->window_class_wide.c_str(), //Window class name
		this->window_title_wide.c_str(), //Window Title
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, //Windows style
		wr.left, //Window X Position
		wr.top, //Window Y Position
		wr.right - wr.left, //Window Width
		wr.bottom - wr.top, //Window Height
		NULL, //Handle to parent of this window. First window
		NULL, //Handle to menu or child window identifier
		this->hInstance, //Handle to the instance of module to be used with this window
		pWindowContainer); //Param to create window

	if (this->handle == NULL)
	{
		ErrorLogger::Log(GetLastError(), "CreateWindowEX Failed for window: " + this->window_title);
		return false;
	}

	//Bring the window up on the screen and set it as main focus
	ShowWindow(this->handle, SW_SHOW);
	SetForegroundWindow(this->handle);
	SetFocus(this->handle);

	return true;
}

bool RenderWindow::ProcessMessages() {

	//Handle the windows message
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG)); //Initialize the message structure

	while (PeekMessage(&msg, //Where to store message
		this->handle, //Handle to window we are checking messages for
		0, //Minimum Filter Msg Value
		0, //Maximum Filter Msg Value
		PM_REMOVE)) //Remove message after capturing it via PeakMessage 
	{
		TranslateMessage(&msg); //Translate message from virtual key messages into character messages
		DispatchMessage(&msg); //Dispatch message to our Window Proc for this window
	}

	//Check if the window was closed
	if (msg.message == WM_NULL)
	{
		if (!IsWindow(this->handle))
		{
			this->handle = NULL; //Message processing loop takes care of destroying this window
			UnregisterClass(this->window_class_wide.c_str(), this->hInstance);
			return false;
		}
	}

	return true;
}

HWND RenderWindow::GetHWND() const
{
	return this->handle;
}

RenderWindow::~RenderWindow()
{
	if (this->handle != NULL)
	{
		UnregisterClass(this->window_class_wide.c_str(), this->hInstance);
		DestroyWindow(handle);
	}
}

LRESULT CALLBACK HandleMsgRedirect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg){

	case WM_CLOSE: {
		//Get ptr to window class
		WindowContainer* const pWindow = reinterpret_cast<WindowContainer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		pWindow->Destroy();
		DestroyWindow(hwnd);
		return 0;
	}
	default:
	{
		//Get ptr to window class
		WindowContainer* const pWindow = reinterpret_cast<WindowContainer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		//Forward message to window class handler
		return pWindow->WindowProc(hwnd, uMsg, wParam, lParam);
	}
	}
}

LRESULT CALLBACK HandleMessageSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
	case WM_NCCREATE: 
	{
		OutputDebugStringA("The window was created.\n");
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		WindowContainer* pWindow = reinterpret_cast<WindowContainer*>(pCreate->lpCreateParams); //Getting WindowContainer
		if (pWindow == nullptr) 
		{
			//Just in case check
			ErrorLogger::Log("Critical Error: Pointer to window container is null during WM_NCCREATE.");
			exit(-1);
		}
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HandleMsgRedirect));
		return pWindow->WindowProc(hwnd, uMsg, wParam, lParam);
	}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

void RenderWindow::RegisterWindowClass()
{
	WNDCLASSEX wc; //Our Window Class
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; //Flags
	wc.lpfnWndProc = HandleMessageSetup; //Pointer to Window Proc function for handling messages from this window
	wc.cbClsExtra = 0; //# of extra bytes to allocate following the window-class structure
	wc.cbWndExtra = 0; //# of extra bytes to allocate following the window instance
	wc.hInstance = this->hInstance; //Handle to the instance that contains the Window Procedure
	wc.hIcon = NULL; //Handle to the class icon. Must be a handle to an icon resource
	wc.hIconSm = NULL; //Handle to small icon for this class
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); //Default Cursor
	wc.hbrBackground = NULL; //Handle to the class background brush for the window's background color
	wc.lpszMenuName = NULL; //Pointer to a null terminated character string for the menu
	wc.lpszClassName = this->window_class_wide.c_str(); //Pointer to null terminated string of our class name for this window
	wc.cbSize = sizeof(WNDCLASSEX); //Need to fill in the size of our struct for cbSize
	RegisterClassEx(&wc); //Register the class so that it is usable
}