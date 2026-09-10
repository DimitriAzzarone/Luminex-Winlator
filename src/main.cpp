#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shellapi.h>

static void openSite(HWND window, const wchar_t* url) {
    HINSTANCE result = ShellExecuteW(window, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
    if ((INT_PTR)result <= 32) {
        MessageBoxW(window,
            L"Installa un browser Windows nel container Winlator e impostalo come predefinito.",
            L"Luminex", MB_OK | MB_ICONINFORMATION);
    }
}

static LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_COMMAND) {
        if (LOWORD(wParam) == 1) openSite(window, L"https://www.google.com/");
        if (LOWORD(wParam) == 2) openSite(window, L"https://drive.google.com/");
        return 0;
    }
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    const wchar_t className[] = L"LuminexWindow";
    WNDCLASSW wc{};
    wc.lpfnWndProc = windowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = className;
    RegisterClassW(&wc);

    HWND window = CreateWindowExW(0, className, L"Luminex",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 360, 170,
        nullptr, nullptr, instance, nullptr);
    if (!window) return 1;

    CreateWindowW(L"BUTTON", L"Google", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        30, 45, 135, 45, window, (HMENU)1, instance, nullptr);
    CreateWindowW(L"BUTTON", L"Google Drive", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        185, 45, 135, 45, window, (HMENU)2, instance, nullptr);

    ShowWindow(window, show);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
