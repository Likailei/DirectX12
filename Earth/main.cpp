#include "Earth.h"

#define WIDTH 1280
#define HEIGHT 720

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    //windowClass.style = CS_GLOBALCLASS;
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.cbClsExtra = windowClass.cbWndExtra = 0;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    windowClass.lpszClassName = L"DirectX12 Application";
    RegisterClassEx(&windowClass);

    Earth MyEarth(WIDTH, HEIGHT, L"Mycraft");
    MyEarth.m_hwnd = CreateWindow(windowClass.lpszClassName,
        MyEarth.GetTitle(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WIDTH,
        HEIGHT,
        nullptr,
        nullptr,
        hInstance,
        &MyEarth);

    MyEarth.OnInit();

    //AllocConsole();

    ShowWindow(MyEarth.m_hwnd, nShowCmd);

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            MyEarth.OnUpdate();
            MyEarth.OnRender();
        }
    }

    return static_cast<char>(msg.wParam);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Earth* pEarth = reinterpret_cast<Earth*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
    {
        // Save the Block* passed in to CreateWindow.
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    return 0;

    /*case WM_KEYDOWN:
        if (pEarth)
        {
            pEarth->OnKeyDown(static_cast<UINT8>(wParam));
        }
        return 0;

    case WM_KEYUP:
        if (pEarth)
        {
            pEarth->OnKeyUp(static_cast<UINT8>(wParam));
        }
        return 0;*/

        /*case WM_PAINT:
            if (pEarth)
            {
                pEarth->OnUpdate();
                pEarth->OnRender();
            }
            return 0;*/

    case WM_RBUTTONDOWN:
        pEarth->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_RBUTTONUP:
        pEarth->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_MOUSEMOVE:
        pEarth->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_MOUSEWHEEL:
        pEarth->OnMWheelRotate(wParam);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}