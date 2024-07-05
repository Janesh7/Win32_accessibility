#pragma once

#include <windows.h>
#include <vector>
#include "Box.h"

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

    const std::vector<Box>& GetBoxes() const { return boxes; }
    RECT GetRect() const { return rect; }

private:
    RECT rect;
    std::vector<Box> boxes;
};
