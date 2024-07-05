#include <windows.h>
#include <oleacc.h>
#include <vector>
#include <string>

// Define STATE_SYSTEM_NORMAL if not defined
#ifndef STATE_SYSTEM_NORMAL
#define STATE_SYSTEM_NORMAL 0x00000000
#endif

class Box // Move the Box class definition here
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

    RECT GetRect() const
    {
        return rect;
    }

    std::wstring GetText() const
    {
        return text;
    }

private:
    RECT rect;
    std::wstring text;
};

class AccessibleBox : public IAccessible
{
public:
    AccessibleBox(Box* box) : refCount(1), box(box)
    {
        CoInitialize(NULL);
    }

    ~AccessibleBox()
    {
        CoUninitialize();
    }

    // IUnknown methods
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
        if (riid == IID_IUnknown || riid == IID_IAccessible)
        {
            *ppvObject = static_cast<IAccessible*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return InterlockedIncrement(&refCount);
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG count = InterlockedDecrement(&refCount);
        if (count == 0)
        {
            delete this;
        }
        return count;
    }

    // IAccessible methods
    HRESULT STDMETHODCALLTYPE get_accParent(IDispatch** ppdispParent) override
    {
        *ppdispParent = NULL;
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE get_accChildCount(long* pcountChildren) override
    {
        *pcountChildren = 0;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE get_accChild(VARIANT varChild, IDispatch** ppdispChild) override
    {
        *ppdispChild = NULL;
        return E_INVALIDARG;
    }

    HRESULT STDMETHODCALLTYPE get_accName(VARIANT varChild, BSTR* pszName) override
    {
        if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
        {
            *pszName = SysAllocString(L"Box");
            return S_OK;
        }
        return E_INVALIDARG;
    }

    HRESULT STDMETHODCALLTYPE get_accValue(VARIANT varChild, BSTR* pszValue) override
    {
        *pszValue = NULL;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE get_accDescription(VARIANT varChild, BSTR* pszDescription) override
    {
        *pszDescription = NULL;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE get_accRole(VARIANT varChild, VARIANT* pvarRole) override
    {
        if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
        {
            pvarRole->vt = VT_I4;
            pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
            return S_OK;
        }
        return E_INVALIDARG;
    }

    HRESULT STDMETHODCALLTYPE get_accState(VARIANT varChild, VARIANT* pvarState) override
    {
        if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
        {
            pvarState->vt = VT_I4;
            pvarState->lVal = STATE_SYSTEM_NORMAL;
            return S_OK;
        }
        return E_INVALIDARG;
    }

    HRESULT STDMETHODCALLTYPE get_accHelp(VARIANT varChild, BSTR* pszHelp) override
    {
        *pszHelp = NULL;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE get_accHelpTopic(BSTR* pszHelpFile, VARIANT varChild, long* pidTopic) override
    {
        *pszHelpFile = NULL;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE get_accKeyboardShortcut(VARIANT varChild, BSTR* pszKeyboardShortcut) override
    {
        *pszKeyboardShortcut = NULL;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE get_accFocus(VARIANT* pvarChild) override
    {
        pvarChild->vt = VT_EMPTY;
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE get_accSelection(VARIANT* pvarChildren) override
    {
        pvarChildren->vt = VT_EMPTY;
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction) override
    {
        *pszDefaultAction = NULL;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE accSelect(long flagsSelect, VARIANT varChild) override
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild) override
    {
        if (varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF)
        {
            *pxLeft = box->GetRect().left;
            *pyTop = box->GetRect().top;
            *pcxWidth = box->GetRect().right - box->GetRect().left;
            *pcyHeight = box->GetRect().bottom - box->GetRect().top;
            return S_OK;
        }
        return E_INVALIDARG;
    }

    HRESULT STDMETHODCALLTYPE accNavigate(long navDir, VARIANT varStart, VARIANT* pvarEndUpAt) override
    {
        pvarEndUpAt->vt = VT_EMPTY;
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE accHitTest(long xLeft, long yTop, VARIANT* pvarChild) override
    {
        pvarChild->vt = VT_EMPTY;
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE accDoDefaultAction(VARIANT varChild) override
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE put_accName(VARIANT varChild, BSTR szName) override
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE put_accValue(VARIANT varChild, BSTR szValue) override
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* pctinfo) override
    {
        *pctinfo = 0;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override
    {
        *ppTInfo = nullptr;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) override
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) override
    {
        return E_NOTIMPL;
    }

private:
    ULONG refCount;
    Box* box;
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

    std::vector<Box>& GetBoxes() // Ensure this returns a reference
    {
        return boxes;
    }

public:
    RECT rect;
private:
    std::vector<Box> boxes;
};

class AccessibleNavbar : public IAccessible
{
public:
    AccessibleNavbar(Navbar* navbar) : refCount(1), navbar(navbar)
    {
        CoInitialize(NULL);
    }

    ~AccessibleNavbar()
    {
        CoUninitialize();
    }

    // IUnknown methods
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
        if (riid == IID_IUnknown || riid == IID_IAccessible)
        {
            *ppvObject = static_cast<IAccessible*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return InterlockedIncrement(&refCount);
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG count = InterlockedDecrement(&refCount);
        if (count == 0)
        {
            delete this;
        }
        return count;
    }

    // IAccessible methods
    HRESULT STDMETHODCALLTYPE get_accParent(IDispatch** ppdispParent) override
    {
        *ppdispParent = NULL;
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE get_accChildCount(long* pcountChildren) override
    {
        *pcountChildren = static_cast<long>(navbar->GetBoxes().size());
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE get_accChild(VARIANT varChild, IDispatch** ppdispChild) override
    {
        if (varChild.vt == VT_I4 && varChild.lVal > 0 && varChild.lVal <= static_cast<long>(navbar->GetBoxes().size()))
        {
            *ppdispChild = new AccessibleBox(&navbar->GetBoxes()[varChild.lVal - 1]);
            return S_OK;
        }
        *ppdispChild = NULL;
        return E_INVALIDARG;
    }

    HRESULT STDMETHODCALLTYPE get_accName(VARIANT varChild, BSTR* pszName) override
    {
        if (varChild.vt == VT_I4)
        {
            if (varChild.lVal == CHILDID_SELF)
            {
                *pszName = SysAllocString(L"Navbar");
                return S_OK;
            }
            else if (varChild.lVal > 0 && varChild.lVal <= static_cast<long>(navbar->GetBoxes().size()))
            {
                *pszName = SysAllocString(navbar->GetBoxes()[varChild.lVal - 1].GetText().c_str());
                return S_OK;
            }
        }
        return E_INVALIDARG;
    }

    HRESULT STDMETHODCALLTYPE get_accValue(VARIANT varChild, BSTR* pszValue) override
    {
        *pszValue = NULL;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE get_accDescription(VARIANT varChild, BSTR* pszDescription) override
    {
        *pszDescription = NULL;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE get_accRole(VARIANT varChild, VARIANT* pvarRole) override
    {
        if (varChild.vt == VT_I4)
        {
            pvarRole->vt = VT_I4;
            if (varChild.lVal == CHILDID_SELF)
            {
                pvarRole->lVal = ROLE_SYSTEM_TOOLBAR;
            }
            else if (varChild.lVal > 0 && varChild.lVal <= static_cast<long>(navbar->GetBoxes().size()))
            {
                pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
            }
            else
            {
                return E_INVALIDARG;
            }
            return S_OK;
        }
        return E_INVALIDARG;
    }

    HRESULT STDMETHODCALLTYPE get_accState(VARIANT varChild, VARIANT* pvarState) override
    {
        if (varChild.vt == VT_I4)
        {
            pvarState->vt = VT_I4;
            pvarState->lVal = STATE_SYSTEM_NORMAL;
            return S_OK;
        }
        return E_INVALIDARG;
    }

    HRESULT STDMETHODCALLTYPE get_accHelp(VARIANT varChild, BSTR* pszHelp) override
    {
        *pszHelp = NULL;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE get_accHelpTopic(BSTR* pszHelpFile, VARIANT varChild, long* pidTopic) override
    {
        *pszHelpFile = NULL;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE get_accKeyboardShortcut(VARIANT varChild, BSTR* pszKeyboardShortcut) override
    {
        *pszKeyboardShortcut = NULL;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE get_accFocus(VARIANT* pvarChild) override
    {
        pvarChild->vt = VT_EMPTY;
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE get_accSelection(VARIANT* pvarChildren) override
    {
        pvarChildren->vt = VT_EMPTY;
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction) override
    {
        *pszDefaultAction = NULL;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE accSelect(long flagsSelect, VARIANT varChild) override
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild) override
    {
        if (varChild.vt == VT_I4)
        {
            if (varChild.lVal == CHILDID_SELF)
            {
                *pxLeft = navbar->rect.left;
                *pyTop = navbar->rect.top;
                *pcxWidth = navbar->rect.right - navbar->rect.left;
                *pcyHeight = navbar->rect.bottom - navbar->rect.top;
                return S_OK;
            }
            else if (varChild.lVal > 0 && varChild.lVal <= static_cast<long>(navbar->GetBoxes().size()))
            {
                RECT boxRect = navbar->GetBoxes()[varChild.lVal - 1].GetRect();
                *pxLeft = boxRect.left;
                *pyTop = boxRect.top;
                *pcxWidth = boxRect.right - boxRect.left;
                *pcyHeight = boxRect.bottom - boxRect.top;
                return S_OK;
            }
        }
        return E_INVALIDARG;
    }

    HRESULT STDMETHODCALLTYPE accNavigate(long navDir, VARIANT varStart, VARIANT* pvarEndUpAt) override
    {
        pvarEndUpAt->vt = VT_EMPTY;
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE accHitTest(long xLeft, long yTop, VARIANT* pvarChild) override
    {
        pvarChild->vt = VT_EMPTY;
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE accDoDefaultAction(VARIANT varChild) override
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE put_accName(VARIANT varChild, BSTR szName) override
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE put_accValue(VARIANT varChild, BSTR szValue) override
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* pctinfo) override
    {
        *pctinfo = 0;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override
    {
        *ppTInfo = nullptr;
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) override
    {
        return E_NOTIMPL;
    }

    HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) override
    {
        return E_NOTIMPL;
    }

private:
    ULONG refCount;
    Navbar* navbar;
};

Navbar* gNavbar;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        RECT navbarRect = { 0, 0, 800, 50 };
        gNavbar = new Navbar(navbarRect);

        RECT box1Rect = { 10, 10, 110, 40 };
        RECT box2Rect = { 120, 10, 220, 40 };
        RECT box3Rect = { 230, 10, 330, 40 };

        Box box1(box1Rect, L"Box 1");
        Box box2(box2Rect, L"Box 2");
        Box box3(box3Rect, L"Box 3");

        gNavbar->AddBox(box1);
        gNavbar->AddBox(box2);
        gNavbar->AddBox(box3);
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        if (gNavbar)
        {
            gNavbar->Draw(hdc);
        }

        EndPaint(hwnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        delete gNavbar;
        gNavbar = nullptr;
        break;
    case WM_GETOBJECT:
    if (lParam == static_cast<LPARAM>(OBJID_CLIENT))
    {
        IAccessible* pAccessible = static_cast<IAccessible*>(new AccessibleNavbar(gNavbar));
        LRESULT lResult = LresultFromObject(IID_IAccessible, wParam, static_cast<IAccessible*>(pAccessible));
        pAccessible->Release();
        return lResult;
    }
    break;

    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    CoInitialize(NULL); // Initialize COM

    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Accessible Navbar",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL)
    {
        CoUninitialize(); // Cleanup COM
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize(); // Cleanup COM
    return 0;
}