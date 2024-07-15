#include <windows.h>
#include "accesskit.h"
#include <vector>
#include <memory>

const WCHAR CLASS_NAME[] = L"AccessKitTest";
const WCHAR WINDOW_TITLE[] = L"Accessible UI";

const accesskit_node_id WINDOW_ID = 0;
const accesskit_node_id BUTTON_1_ID = 1;
const accesskit_node_id BUTTON_2_ID = 2;
const accesskit_node_id BUTTON_3_ID = 4;
const accesskit_node_id NAVBAR_ID = 3;
#define INITIAL_FOCUS BUTTON_1_ID

const accesskit_rect BUTTON_1_RECT = { 20.0, 20.0, 100.0, 60.0 };
const accesskit_rect BUTTON_2_RECT = { 120.0, 20.0, 200.0, 60.0 };
const accesskit_rect BUTTON_3_RECT = { 220.0, 20.0, 300.0, 60.0 };
const accesskit_rect NAVBAR_RECT = { 0.0, 0.0, 320.0, 100.0 };

const uint32_t SET_FOCUS_MSG = WM_USER;
const uint32_t DO_DEFAULT_ACTION_MSG = WM_USER + 1;

class Button {
public:
    accesskit_node_id id;
    const char* name;
    accesskit_rect rect;
    COLORREF color;

    Button(accesskit_node_id id, const char* name, accesskit_rect rect, COLORREF color)
        : id(id), name(name), rect(rect), color(color) {}

    accesskit_node* build() {
        accesskit_node_builder* builder = accesskit_node_builder_new(ACCESSKIT_ROLE_BUTTON);
        accesskit_node_builder_set_bounds(builder, rect);
        accesskit_node_builder_set_name(builder, name);
        accesskit_node_builder_add_action(builder, ACCESSKIT_ACTION_FOCUS);
        accesskit_node_builder_set_default_action_verb(builder, ACCESSKIT_DEFAULT_ACTION_VERB_CLICK);
        return accesskit_node_builder_build(builder);
    }

    void draw(HDC hdc) {
        HBRUSH brush = CreateSolidBrush(color);
        RECT rectToDraw = { (LONG)rect.x0, (LONG)rect.y0, (LONG)rect.x1, (LONG)rect.y1 };
        FillRect(hdc, &rectToDraw, brush);
        SetBkMode(hdc, TRANSPARENT);
        DrawTextA(hdc, name, -1, &rectToDraw, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        DeleteObject(brush);
    }
};

class Navbar {
public:
    accesskit_node_id id;
    accesskit_rect rect;
    std::vector<std::shared_ptr<Button>> buttons;
    COLORREF color;

    Navbar(accesskit_node_id id, accesskit_rect rect, std::vector<std::shared_ptr<Button>> buttons, COLORREF color)
        : id(id), rect(rect), buttons(buttons), color(color) {}

    accesskit_node* build() {
        accesskit_node_builder* builder = accesskit_node_builder_new(ACCESSKIT_ROLE_GROUP);
        accesskit_node_builder_set_bounds(builder, rect);
        accesskit_node_builder_set_name(builder, "Navbar");
        for (const auto& button : buttons) {
            accesskit_node_builder_push_child(builder, button->id);
        }
        return accesskit_node_builder_build(builder);
    }

    void draw(HDC hdc) {
        HBRUSH brush = CreateSolidBrush(color);
        RECT rectToDraw = { (LONG)rect.x0, (LONG)rect.y0, (LONG)rect.x1, (LONG)rect.y1 };
        FillRect(hdc, &rectToDraw, brush);
        DeleteObject(brush);
        for (const auto& button : buttons) {
            button->draw(hdc);
        }
    }
};

struct WindowState {
    accesskit_windows_adapter* adapter;
    accesskit_node_id focus;
    std::shared_ptr<Navbar> navbar;
    std::vector<std::shared_ptr<Button>> buttons;

    WindowState(accesskit_windows_adapter* adapter, accesskit_node_id focus)
        : adapter(adapter), focus(focus) {}

    void addButton(std::shared_ptr<Button> button) {
        buttons.push_back(button);
    }

    void addNavbar(std::shared_ptr<Navbar> navbar) {
        this->navbar = navbar;
    }

    accesskit_node* buildRoot() {
        accesskit_node_builder* builder = accesskit_node_builder_new(ACCESSKIT_ROLE_WINDOW);
        accesskit_node_builder_push_child(builder, NAVBAR_ID);
        return accesskit_node_builder_build(builder);
    }

    accesskit_tree_update* buildInitialTree() {
        accesskit_node* root = buildRoot();
        accesskit_node* navbarNode = navbar->build();
        accesskit_tree_update* update = accesskit_tree_update_with_capacity_and_focus(5, focus);
        accesskit_tree* tree = accesskit_tree_new(WINDOW_ID);
        accesskit_tree_set_app_name(tree, "Hello World");
        accesskit_tree_update_set_tree(update, tree);
        accesskit_tree_update_push_node(update, WINDOW_ID, root);
        accesskit_tree_update_push_node(update, NAVBAR_ID, navbarNode);
        for (const auto& button : buttons) {
            accesskit_tree_update_push_node(update, button->id, button->build());
        }
        return update;
    }

    void draw(HDC hdc) {
        if (navbar) {
            navbar->draw(hdc);
        }
    }
};

void windowStateFree(WindowState* state) {
    accesskit_windows_adapter_free(state->adapter);
    delete state;
}

void windowStateSetFocus(WindowState* state, accesskit_node_id focus) {
    state->focus = focus;
    accesskit_windows_queued_events* events =
        accesskit_windows_adapter_update_if_active(state->adapter, [](void* userdata) {
        WindowState* state = static_cast<WindowState*>(userdata);
        accesskit_tree_update* update = accesskit_tree_update_with_focus(state->focus);
        return update;
            }, state);
    if (events != NULL) {
        accesskit_windows_queued_events_raise(events);
    }
}

void windowStatePressButton(WindowState* state, accesskit_node_id id) {
    // Your custom logic here
    MessageBox(NULL, (id == BUTTON_1_ID ? L"Button 1 pressed" : (id == BUTTON_2_ID ? L"Button 2 pressed" : L"Button 3 pressed")), L"Button Pressed", MB_OK);
}

WindowState* getWindowState(HWND window) {
    return reinterpret_cast<WindowState*>(GetWindowLongPtr(window, GWLP_USERDATA));
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        accesskit_node_id* initialFocus = reinterpret_cast<accesskit_node_id*>(createStruct->lpCreateParams);
        WindowState* state = new WindowState(
            accesskit_windows_adapter_new(hwnd, false, [](accesskit_action_request* request, void* userdata) {
                HWND window = reinterpret_cast<HWND>(userdata);
                if (request->action == ACCESSKIT_ACTION_FOCUS) {
                    PostMessage(window, SET_FOCUS_MSG, 0, static_cast<LPARAM>(request->target));
                }
                else if (request->action == ACCESSKIT_ACTION_DEFAULT) {
                    PostMessage(window, DO_DEFAULT_ACTION_MSG, 0, static_cast<LPARAM>(request->target));
                }
                accesskit_action_request_free(request);
                }, hwnd),
            *initialFocus);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    else if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        WindowState* state = getWindowState(hwnd);
        if (state) {
            state->draw(hdc);
        }
        EndPaint(hwnd, &ps);
    }
    else if (msg == WM_DESTROY) {
        LONG_PTR ptr = SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        if (ptr != 0) {
            WindowState* state = reinterpret_cast<WindowState*>(ptr);
            windowStateFree(state);
        }
        PostQuitMessage(0);
    }
    else if (msg == WM_GETOBJECT) {
        WindowState* state = getWindowState(hwnd);
        if (state == NULL) {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        accesskit_opt_lresult result =
            accesskit_windows_adapter_handle_wm_getobject(
                state->adapter, wParam, lParam, [](void* userdata) {
                    WindowState* state = static_cast<WindowState*>(userdata);
                    return state->buildInitialTree();
                }, state);
        if (result.has_value) {
            return result.value;
        }
        else {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }
    else if (msg == WM_SETFOCUS || msg == WM_EXITMENULOOP || msg == WM_EXITSIZEMOVE) {
        accesskit_windows_queued_events* events =
            accesskit_windows_adapter_update_window_focus_state(getWindowState(hwnd)->adapter, true);
        if (events != NULL) {
            accesskit_windows_queued_events_raise(events);
        }
    }
    else if (msg == WM_KILLFOCUS || msg == WM_ENTERMENULOOP || msg == WM_ENTERSIZEMOVE) {
        accesskit_windows_queued_events* events =
            accesskit_windows_adapter_update_window_focus_state(getWindowState(hwnd)->adapter, false);
        if (events != NULL) {
            accesskit_windows_queued_events_raise(events);
        }
    }
    else if (msg == WM_KEYDOWN) {
        WindowState* state = getWindowState(hwnd);
        if (wParam == VK_TAB) {
            accesskit_node_id newFocus;
            if (state->focus == BUTTON_1_ID) {
                newFocus = BUTTON_2_ID;
            }
            else if (state->focus == BUTTON_2_ID) {
                newFocus = BUTTON_3_ID;
            }
            else {
                newFocus = BUTTON_1_ID;
            }
            windowStateSetFocus(state, newFocus);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        else if (wParam == VK_SPACE) {
            windowStatePressButton(state, state->focus);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        else {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }
    else if (msg == SET_FOCUS_MSG) {
        accesskit_node_id id = static_cast<accesskit_node_id>(lParam);
        windowStateSetFocus(getWindowState(hwnd), id);
        InvalidateRect(hwnd, NULL, TRUE);
    }
    else if (msg == DO_DEFAULT_ACTION_MSG) {
        accesskit_node_id id = static_cast<accesskit_node_id>(lParam);
        windowStatePressButton(getWindowState(hwnd), id);
        InvalidateRect(hwnd, NULL, TRUE);
    }
    else {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HWND createWindow(const WCHAR* title, accesskit_node_id initialFocus) {
    return CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_NAME, title, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, GetModuleHandle(NULL), &initialFocus);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_NAME;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        return 0;
    }

    hwnd = createWindow(WINDOW_TITLE, INITIAL_FOCUS);

    if (hwnd == NULL) {
        return 0;
    }

    std::shared_ptr<Button> button1 = std::make_shared<Button>(BUTTON_1_ID, "Button 1", BUTTON_1_RECT, RGB(200, 200, 200));
    std::shared_ptr<Button> button2 = std::make_shared<Button>(BUTTON_2_ID, "Button 2", BUTTON_2_RECT, RGB(200, 200, 200));
    std::shared_ptr<Button> button3 = std::make_shared<Button>(BUTTON_3_ID, "Button 3", BUTTON_3_RECT, RGB(200, 200, 200));
    std::vector<std::shared_ptr<Button>> buttons = { button1, button2, button3 };
    std::shared_ptr<Navbar> navbar = std::make_shared<Navbar>(NAVBAR_ID, NAVBAR_RECT, buttons, RGB(0, 0, 255));
    WindowState* state = getWindowState(hwnd);
    state->addNavbar(navbar);
    state->addButton(button1);
    state->addButton(button2);
    state->addButton(button3);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}
