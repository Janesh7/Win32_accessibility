#include <windows.h>
#include "accesskit.h"

const WCHAR CLASS_NAME[] = L"AccessKitCustomUI";
const WCHAR WINDOW_TITLE[] = L"Custom UI Accessibility";

const accesskit_node_id WINDOW_ID = 0;
const accesskit_node_id NAVBAR_ID = 1;
const accesskit_node_id BUTTON_1_ID = 2;
const accesskit_node_id BUTTON_2_ID = 3;
const accesskit_node_id BUTTON_3_ID = 4;

const accesskit_rect NAVBAR_RECT = { 0.0, 0.0, 400.0, 150.0 };
const accesskit_rect BUTTON_1_RECT = { 20.0, 60.0, 120.0, 110.0 };
const accesskit_rect BUTTON_2_RECT = { 140.0, 60.0, 240.0, 110.0 };
const accesskit_rect BUTTON_3_RECT = { 260.0, 60.0, 360.0, 110.0 };

const uint32_t SET_FOCUS_MSG = WM_USER;
const uint32_t DO_DEFAULT_ACTION_MSG = WM_USER + 1;

accesskit_node* build_node(accesskit_node_id id, const char* name, accesskit_rect rect, accesskit_role role) {
    accesskit_node_builder* builder = accesskit_node_builder_new(role);
    accesskit_node_builder_set_bounds(builder, rect);
    accesskit_node_builder_set_name(builder, name);
    accesskit_node_builder_add_action(builder, ACCESSKIT_ACTION_FOCUS);
    if (role == ACCESSKIT_ROLE_BUTTON) {
        accesskit_node_builder_set_default_action_verb(builder, ACCESSKIT_DEFAULT_ACTION_VERB_CLICK);
    }
    return accesskit_node_builder_build(builder);
}

struct window_state {
    accesskit_windows_adapter* adapter;
    accesskit_node_id focus;
};

void window_state_free(struct window_state* state) {
    accesskit_windows_adapter_free(state->adapter);
    free(state);
}

accesskit_node* window_state_build_root(struct window_state* state) {
    accesskit_node_builder* builder = accesskit_node_builder_new(ACCESSKIT_ROLE_WINDOW);
    accesskit_node_builder_push_child(builder, NAVBAR_ID);
    return accesskit_node_builder_build(builder);
}

accesskit_node* build_navbar_node(struct window_state* state) {
    accesskit_node_builder* builder = accesskit_node_builder_new(ACCESSKIT_ROLE_GROUP);
    accesskit_node_builder_set_bounds(builder, NAVBAR_RECT);
    accesskit_node_builder_set_name(builder, "Navbar");
    accesskit_node_builder_push_child(builder, BUTTON_1_ID);
    accesskit_node_builder_push_child(builder, BUTTON_2_ID);
    accesskit_node_builder_push_child(builder, BUTTON_3_ID);
    return accesskit_node_builder_build(builder);
}

accesskit_tree_update* build_initial_tree(void* userdata) {
    struct window_state* state = userdata;
    accesskit_node* root = window_state_build_root(state);
    accesskit_node* navbar = build_navbar_node(state);
    accesskit_node* button_1 = build_node(BUTTON_1_ID, "Button 1", BUTTON_1_RECT, ACCESSKIT_ROLE_BUTTON);
    accesskit_node* button_2 = build_node(BUTTON_2_ID, "Button 2", BUTTON_2_RECT, ACCESSKIT_ROLE_BUTTON);
    accesskit_node* button_3 = build_node(BUTTON_3_ID, "Button 3", BUTTON_3_RECT, ACCESSKIT_ROLE_BUTTON);

    accesskit_tree_update* result = accesskit_tree_update_with_capacity_and_focus(5, state->focus);
    accesskit_tree* tree = accesskit_tree_new(WINDOW_ID);
    accesskit_tree_set_app_name(tree, "Custom UI Accessibility");
    accesskit_tree_update_set_tree(result, tree);
    accesskit_tree_update_push_node(result, WINDOW_ID, root);
    accesskit_tree_update_push_node(result, NAVBAR_ID, navbar);
    accesskit_tree_update_push_node(result, BUTTON_1_ID, button_1);
    accesskit_tree_update_push_node(result, BUTTON_2_ID, button_2);
    accesskit_tree_update_push_node(result, BUTTON_3_ID, button_3);
    return result;
}

void do_action(accesskit_action_request* request, void* userdata) {
    HWND window = userdata;
    if (request->action == ACCESSKIT_ACTION_FOCUS) {
        LPARAM lparam = (LPARAM)(request->target);
        PostMessage((HWND)window, SET_FOCUS_MSG, 0, lparam);
    }
    else if (request->action == ACCESSKIT_ACTION_DEFAULT) {
        LPARAM lparam = (LPARAM)(request->target);
        PostMessage((HWND)window, DO_DEFAULT_ACTION_MSG, 0, lparam);
    }
    accesskit_action_request_free(request);
}

accesskit_tree_update* build_tree_update_for_focus_update(void* userdata) {
    struct window_state* state = userdata;
    accesskit_tree_update* update = accesskit_tree_update_with_focus(state->focus);
    return update;
}

void window_state_set_focus(struct window_state* state, accesskit_node_id focus) {
    state->focus = focus;
    accesskit_windows_queued_events* events = accesskit_windows_adapter_update_if_active(
        state->adapter, build_tree_update_for_focus_update, state);
    if (events != NULL) {
        accesskit_windows_queued_events_raise(events);
    }
}

struct window_state* get_window_state(HWND window) {
    return (struct window_state*)(GetWindowLongPtr(window, GWLP_USERDATA));
}

void update_window_focus_state(HWND window, bool is_focused) {
    struct window_state* state = get_window_state(window);
    accesskit_windows_queued_events* events = accesskit_windows_adapter_update_window_focus_state(state->adapter, is_focused);
    if (events != NULL) {
        accesskit_windows_queued_events_raise(events);
    }
}

struct window_create_params {
    accesskit_node_id initial_focus;
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* create_struct = (CREATESTRUCT*)lParam;
        struct window_create_params* create_params = (struct window_create_params*)create_struct->lpCreateParams;
        struct window_state* state = malloc(sizeof(struct window_state));
        state->adapter = accesskit_windows_adapter_new(hwnd, false, do_action, (void*)hwnd);
        state->focus = create_params->initial_focus;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)state);
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    else if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Draw Navbar
        HBRUSH navbarBrush = CreateSolidBrush(RGB(0, 0, 255));
        RECT navbarRect = { NAVBAR_RECT.x0, NAVBAR_RECT.y0, NAVBAR_RECT.x1, NAVBAR_RECT.y1 };
        FillRect(hdc, &navbarRect, navbarBrush);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        DrawText(hdc, L"Navbar", -1, &navbarRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        DeleteObject(navbarBrush);

        // Draw Buttons
        HBRUSH buttonBrush = CreateSolidBrush(RGB(200, 200, 200));

        RECT button1Rect = { BUTTON_1_RECT.x0, BUTTON_1_RECT.y0, BUTTON_1_RECT.x1, BUTTON_1_RECT.y1 };
        FillRect(hdc, &button1Rect, buttonBrush);
        DrawText(hdc, L"Button 1", -1, &button1Rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        RECT button2Rect = { BUTTON_2_RECT.x0, BUTTON_2_RECT.y0, BUTTON_2_RECT.x1, BUTTON_2_RECT.y1 };
        FillRect(hdc, &button2Rect, buttonBrush);
        DrawText(hdc, L"Button 2", -1, &button2Rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        RECT button3Rect = { BUTTON_3_RECT.x0, BUTTON_3_RECT.y0, BUTTON_3_RECT.x1, BUTTON_3_RECT.y1 };
        FillRect(hdc, &button3Rect, buttonBrush);
        DrawText(hdc, L"Button 3", -1, &button3Rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        DeleteObject(buttonBrush);

        EndPaint(hwnd, &ps);
    }
    else if (msg == WM_DESTROY) {
        LONG_PTR ptr = SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
        if (ptr != 0) {
            struct window_state* state = (struct window_state*)ptr;
            window_state_free(state);
        }
        PostQuitMessage(0);
    }
    else if (msg == WM_GETOBJECT) {
        struct window_state* state = get_window_state(hwnd);
        if (state == NULL) {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        accesskit_opt_lresult result = accesskit_windows_adapter_handle_wm_getobject(
            state->adapter, wParam, lParam, build_initial_tree, state);
        if (result.has_value) {
            return result.value;
        }
        else {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
    }
    else if (msg == WM_SETFOCUS || msg == WM_ACTIVATEAPP) {
        update_window_focus_state(hwnd, true);
    }
    else if (msg == WM_KILLFOCUS || msg == WM_ENTERMENULOOP || msg == WM_ENTERSIZEMOVE) {
        update_window_focus_state(hwnd, false);
    }
    else if (msg == SET_FOCUS_MSG) {
        struct window_state* state = get_window_state(hwnd);
        window_state_set_focus(state, (accesskit_node_id)lParam);
        InvalidateRect(hwnd, NULL, TRUE);
    }
    else if (msg == WM_KEYDOWN) {
        struct window_state* state = get_window_state(hwnd);
        if (wParam == VK_TAB) {
            if (state->focus == NAVBAR_ID) {
                window_state_set_focus(state, BUTTON_1_ID);
            }
            else if (state->focus == BUTTON_1_ID) {
                window_state_set_focus(state, BUTTON_2_ID);
            }
            else if (state->focus == BUTTON_2_ID) {
                window_state_set_focus(state, BUTTON_3_ID);
            }
            else if (state->focus == BUTTON_3_ID) {
                window_state_set_focus(state, NAVBAR_ID);
            }
        }
    }
    else {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HWND create_window(const WCHAR* title, accesskit_node_id initial_focus) {
    struct window_create_params create_params;
    create_params.initial_focus = initial_focus;

    return CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_NAME, title,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        400, 200, NULL, NULL,
        GetModuleHandle(NULL), &create_params);
}

int main() {
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_NAME;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        return 0;
    }

    hwnd = create_window(WINDOW_TITLE, NAVBAR_ID);

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}
