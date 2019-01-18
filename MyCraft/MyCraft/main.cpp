#include "Game.h"

#define WIDTH 1280
#define HEIGHT 720

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_GLOBALCLASS;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.cbClsExtra = windowClass.cbWndExtra = 0;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	windowClass.lpszClassName = L"DirectX12 Application";
	RegisterClassEx(&windowClass);

	Game MyGame(WIDTH, HEIGHT, L"Mycraft");
	MyGame.m_hwnd = CreateWindow(windowClass.lpszClassName,
		MyGame.GetTitle(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		WIDTH,
		HEIGHT,
		nullptr,
		nullptr,
		hInstance,
		&MyGame);
	
	MyGame.OnInit();

	//AllocConsole();

	ShowWindow(MyGame.m_hwnd, nShowCmd);

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return static_cast<char>(msg.wParam);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Game* pGame = reinterpret_cast<Game*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_CREATE:
	{
		// Save the Cube* passed in to CreateWindow.
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	}
	return 0;

	/*case WM_KEYDOWN:
		if (pGame)
		{
			pGame->OnKeyDown(static_cast<UINT8>(wParam));
		}
		return 0;

	case WM_KEYUP:
		if (pGame)
		{
			pGame->OnKeyUp(static_cast<UINT8>(wParam));
		}
		return 0;*/

	case WM_PAINT:
		if (pGame)
		{
			pGame->OnUpdate();
			pGame->OnRender();
		}
		return 0;

	case WM_RBUTTONDOWN:
		pGame->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_RBUTTONUP:
		pGame->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEMOVE:
		pGame->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEWHEEL:
		pGame->OnMWheelRotate(wParam);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
