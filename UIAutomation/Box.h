#pragma once

#include <windows.h>
#include <ole2.h>
#include <uiautomation.h>
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

    RECT GetRect() const { return rect; }
    std::wstring GetText() const { return text; }

private:
    RECT rect;
    std::wstring text;
};
