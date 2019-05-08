#include "Precompiled.h"

auto OnKeyDown(HWND window, WPARAM wparam) -> void
{
    if (VK_LEFT != wparam && VK_RIGHT != wparam) return;

    auto alpha = BYTE {};

    VERIFY(GetLayeredWindowAttributes(window,
                                      nullptr,
                                      &alpha,
                                      nullptr));

    if (VK_LEFT == wparam)
    {
        alpha = max(0, alpha - 10);
    }
    else
    {
        alpha = min(255, alpha + 10);
    }

    VERIFY(SetLayeredWindowAttributes(window,
                                      0,
                                      alpha,
                                      LWA_ALPHA));

    wchar_t text[100];
    swprintf_s(text, L"Sample %.f%%", alpha / 255.0f * 100.0f);

    SetWindowText(window, text);
}

auto __stdcall wWinMain(HINSTANCE module, HINSTANCE, PWSTR, int) -> int
{
    auto wc = WNDCLASS {};
    wc.lpszClassName = L"window";
    wc.hInstance = module;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    wc.lpfnWndProc = [](HWND window,
                        UINT message,
                        WPARAM wparam,
                        LPARAM lparam) -> LRESULT
    {
        if (WM_KEYDOWN == message)
        {
            OnKeyDown(window, wparam);
            return 0;
        }

        if (WM_DESTROY == message)
        {
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(window, message, wparam, lparam);
    };

    RegisterClass(&wc);

    auto window = CreateWindowEx(WS_EX_LAYERED,
                 wc.lpszClassName,
                 L"Sample",
                 WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                 CW_USEDEFAULT,
                 CW_USEDEFAULT,
                 CW_USEDEFAULT,
                 CW_USEDEFAULT,
                 nullptr,
                 nullptr,
                 module,
                 nullptr);

    VERIFY(SetLayeredWindowAttributes(window,
                                      0,
                                      255,
                                      LWA_ALPHA));

    auto message = MSG {};

    while (GetMessage(&message, nullptr, 0, 0))
    {
        DispatchMessage(&message);
    }
}
