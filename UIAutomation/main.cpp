#include <windows.h>
#include <ole2.h>
#include <uiautomation.h>
#include "Navbar.h"
#include "NavbarProvider.h"
#include <iostream>

Navbar* gNavbar;
NavbarProvider* gNavbarProvider;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;

    switch (msg)
    {
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        gNavbar->Draw(hdc);
        EndPaint(hwnd, &ps);
        break;

    case WM_GETOBJECT:
        if (lParam == UiaRootObjectId)
        {
            std::cout << "WM_GETOBJECT received" << std::endl;
            return UiaReturnRawElementProvider(hwnd, wParam, lParam, gNavbarProvider);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;

    RECT navbarRect = { 0, 0, 400, 100 };
    gNavbar = new Navbar(navbarRect);
    gNavbar->AddBox(Box({ 10, 10, 90, 60 }, L"Button 1"));
    gNavbar->AddBox(Box({ 110, 10, 190, 60 }, L"Button 2"));
    gNavbar->AddBox(Box({ 210, 10, 290, 60 }, L"Button 3"));

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"MyWindowClass";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        TEXT("Accessible Navbar"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        std::cerr << "Failed to create window!" << std::endl;
        return 0;
    }

    gNavbarProvider = new NavbarProvider(gNavbar, hwnd);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    delete gNavbar;
    gNavbarProvider->Release();

    return (int)msg.wParam;
}
