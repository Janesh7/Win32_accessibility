#include <windows.h>
#include <vector>
#include <string>
#include <atlbase.h>
#include <atlcom.h>

// Helper function to set accessible name using Windows API
void SetAccessibleName(HWND hwnd, const std::wstring& name) {
	SetWindowText(hwnd, name.c_str());
}

class Box {
public:
	Box(RECT rect, const std::wstring& text) : rect(rect), text(text), hwnd(nullptr) {}

	void Draw(HDC hdc, HWND parentHwnd) {
		// Create a white brush for the boxes
		HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
		FillRect(hdc, &rect, hBrush);
		DeleteObject(hBrush);

		// Create a font for the text
		HFONT hFont = CreateFont(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
		HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

		// Set text color to black
		SetTextColor(hdc, RGB(0, 0, 0));
		SetBkMode(hdc, TRANSPARENT); // Set background mode to transparent

		// Draw the text in the center of the box
		DrawText(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		// Restore the old font
		SelectObject(hdc, hOldFont);
		DeleteObject(hFont);

		// Create an off-screen window to host the accessible object for this box
		if (!hwnd) {
			hwnd = CreateWindowEx(
				WS_EX_TRANSPARENT, TEXT("STATIC"), NULL,
				WS_CHILD | WS_VISIBLE,
				rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
				parentHwnd, NULL, GetModuleHandle(NULL), NULL);
			SetAccessibleName(hwnd, text);
		}
	}

private:
	RECT rect;
	std::wstring text;
	HWND hwnd;
};

class Navbar {
public:
	Navbar(RECT rect) : rect(rect), hwnd(nullptr) {}

	void AddBox(const Box& box) {
		boxes.push_back(box);
	}

	void Draw(HDC hdc, HWND parentHwnd) {
		// Create a blue brush for the navbar
		HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 255));
		FillRect(hdc, &rect, hBrush);
		DeleteObject(hBrush);

		// Create an off-screen window to host the accessible object for the navbar
		if (!hwnd) {
			hwnd = CreateWindowEx(
				WS_EX_TRANSPARENT, TEXT("STATIC"), NULL,
				WS_CHILD | WS_VISIBLE,
				rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
				parentHwnd, NULL, GetModuleHandle(NULL), NULL);
			SetAccessibleName(hwnd, L"Navbar");
		}

		// Draw each box
		for (auto& box : boxes) {
			box.Draw(hdc, parentHwnd);
		}
	}

private:
	RECT rect;
	std::vector<Box> boxes;
	HWND hwnd;
};

// Global instance of Navbar
Navbar* gNavbar;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;

	switch (msg) {
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		// Draw the navbar and boxes
		gNavbar->Draw(hdc, hwnd);

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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	(void)hPrevInstance; // To avoid warning for unused parameter
	(void)lpCmdLine;     // To avoid warning for unused parameter

	// Define the rectangle for the main window (navbar)
	RECT navbarRect = { 0, 0, 400, 100 };
	gNavbar = new Navbar(navbarRect);

	// Define the boxes and add them to the navbar
	gNavbar->AddBox(Box({ 10, 10, 90, 60 }, L"Button 1"));
	gNavbar->AddBox(Box({ 110, 10, 190, 60 }, L"Button 2"));
	gNavbar->AddBox(Box({ 210, 10, 290, 60 }, L"Button 3"));

	// Register window class
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("MyWindowClass");
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	RegisterClass(&wc);

	// Create the main window
	HWND hwnd = CreateWindowEx(
		0,
		wc.lpszClassName,
		TEXT("Rectangle with Boxes"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
		NULL, NULL, hInstance, NULL);

	if (hwnd == NULL) {
		return 0;
	}

	// Show and update the main window
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	// Message loop
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Cleanup
	delete gNavbar;

	return (int)msg.wParam;
}