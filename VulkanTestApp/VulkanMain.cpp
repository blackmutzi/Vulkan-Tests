#include "stdafx.h"
#include "Vulkan.h"
#include "GraphicManager.h"

using namespace Graphic;

// Global
Vulkan vulkan_application;

//declarations 
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void create_window( Win32Surface & surface );


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		vulkan_application.run();
		break;
	case WM_GETMINMAXINFO: // set window's minimum size
		//((MINMAXINFO *)lParam)->ptMinTrackSize = demo.minsize;
		return 0;
	case WM_SIZE:
		// Resize the application to the new window size, except when
		// it was minimized. Vulkan doesn't support images or swapchains
		// with width=0 and height=0.
		if (wParam != SIZE_MINIMIZED) 
		{
			GraphicManager::getSingletonPtr()->getSurface().width = lParam & 0xffff;
			GraphicManager::getSingletonPtr()->getSurface().height = (lParam & 0xffff0000) >> 16;

			/* Ein kleinen Bug gibt es noch bei 3x Resize ...*/
			vulkan_application.resize();
		}
		break;
	default:
		break;
	}

	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void create_window( Win32Surface & surface ) 
{
	WNDCLASSEX win_class;

	// Initialize the window class structure:
	win_class.cbSize = sizeof(WNDCLASSEX);
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = WndProc;
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = surface.connection; // hInstance
	win_class.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	win_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	win_class.lpszMenuName = nullptr;
	win_class.lpszClassName = surface.window_title;
	win_class.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

	// Register window class:
	if (!RegisterClassEx(&win_class)) {
		// It didn't work, so try to give a useful error:
		printf("Unexpected error trying to start the application!\n");
		fflush(stdout);
		exit(1);
	}

	// Create window with the registered class:
	RECT wr = { 0, 0, static_cast<LONG>(surface.width), static_cast<LONG>(surface.height) };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	surface.window = CreateWindowEx(0,
		surface.window_title,  // class name
		surface.window_title, // app name
		WS_OVERLAPPEDWINDOW | // window style
		WS_VISIBLE | WS_SYSMENU,
		100, 100,           // x/y coords
		wr.right - wr.left, // width
		wr.bottom - wr.top, // height
		nullptr,            // handle to parent
		nullptr,            // handle to menu
		surface.connection,         // hInstance
		nullptr);           // no extra parameters

	if (!surface.window) {
		// It didn't work, so try to give a useful error:
		printf("Cannot create a window in which to draw!\n");
		fflush(stdout);
		exit(1);
	}

	// Window client area size must be at least 1 pixel high, to prevent
	// crash.
	//minsize.x = GetSystemMetrics(SM_CXMINTRACK);
	//minsize.y = GetSystemMetrics(SM_CYMINTRACK) + 1;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine,int nCmdShow) 
{
	MSG msg;   // message
	bool done; // flag saying when app is complete

	Win32Surface surface;
	surface.connection = hInstance;
	surface.width = 800;
	surface.height = 600;
	surface.window_title = "VulkanTestApp";

	// create window 
	create_window( surface );
	GraphicManager::getSingletonPtr()->createInstance();
	GraphicManager::getSingletonPtr()->createSurface(&surface);

	// init Vulkan Application
	vulkan_application.init();
	// prepare Vulkan Application 
	vulkan_application.prepare();

	done = false; // initialize loop condition variable
			  
	// main message loop
	while (!done) 
	{
		PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
		
		if (msg.message == WM_QUIT) // check for a quit message
		{
			done = true; // if found, quit app
		}
		else 
		{
			/* Translate and dispatch to event queue*/
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		RedrawWindow( surface.window , nullptr, nullptr, RDW_INTERNALPAINT);
	}

	vulkan_application.cleanup();

	return (int)msg.wParam;
}
