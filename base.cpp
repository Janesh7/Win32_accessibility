#include <windows.h>
#include <vector>
#include <string>
 
class Box
{
public:
Box(RECT rect, const std::wstring& text) : rect(rect), text(text) {}
void Draw(HDC hdc)
{
// Create a white brush for the boxes
HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
FillRect(hdc, &rect, hBrush);
DeleteObject(hBrush);
 
// Create a font for the text
HFONT hFont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
// Set text color to black
SetTextColor(hdc, RGB(0, 0, 0));
 
// Draw the text in the center of the box
DrawText(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
 
// Restore the old font
SelectObject(hdc, hOldFont);
DeleteObject(hFont);
}
private:
RECT rect;
std::wstring text;
};
 
class Navbar
{
public:
Navbar(RECT rect) : rect(rect) {}
 
void AddBox(const Box& box)
{
boxes.push_back(box);
}
 
void Draw(HDC hdc)
{
// Create a blue brush for the navbar
HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 255));
FillRect(hdc, &rect, hBrush);
DeleteObject(hBrush);
 
// Draw each box
for (auto& box : boxes)
{
box.Draw(hdc);
}
}
private:
RECT rect;
std::vector<Box> boxes;
};
 
// Global instance of Navbar
Navbar* gNavbar;
 
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
HDC hdc;
PAINTSTRUCT ps;
 
switch (msg)
{
case WM_PAINT:
hdc = BeginPaint(hwnd, &ps);
 
// Draw the navbar and boxes
gNavbar->Draw(hdc);
 
EndPaint(hwnd, &ps);
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
(void)hPrevInstance; // To avoid warning for unused parameter
(void)lpCmdLine;     // To avoid warning for unused parameter
 
// Define the rectangle for the navbar
RECT navbarRect = { 0, 0, 400, 100 };
gNavbar = new Navbar(navbarRect);
 
// Define the boxes and add them to the navbar
gNavbar->AddBox(Box({ 10, 10, 90, 60 }, L"Button 1"));
gNavbar->AddBox(Box({ 110, 10, 190, 60 }, L"Button 2"));
gNavbar->AddBox(Box({ 210, 10, 290, 60 }, L"Button 3"));
 
WNDCLASS wc = { 0 };
wc.lpfnWndProc = WndProc;
wc.hInstance = hInstance;
wc.lpszClassName = L"MyWindowClass";
wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
 
MSG msg = { 0 };
 
RegisterClass(&wc);
 
HWND hwnd = CreateWindowEx(
0,
wc.lpszClassName,
TEXT("Rectangle with Boxes"),
WS_OVERLAPPEDWINDOW,
CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
NULL, NULL, hInstance, NULL);
 
if (hwnd == NULL)
{
return 0;
}
 
ShowWindow(hwnd, nCmdShow);
UpdateWindow(hwnd);
 
while (GetMessage(&msg, NULL, 0, 0))
{
TranslateMessage(&msg);
DispatchMessage(&msg);
}
 
// Cleanup
delete gNavbar;
 
return (int)msg.wParam;
}