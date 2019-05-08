#include "Precompiled.h"

auto OnSize(HWND window, WPARAM wparam) -> void
{
    if (SIZE_MINIMIZED != wparam)
    {
        auto rect = RECT {};
        VERIFY(GetWindowRect(window, &rect));

        auto region = CreateRoundRectRgn(0,
                                         0,
                                         rect.right - rect.left,
                                         rect.bottom - rect.top,
                                         100, 
                                         100);

        VERIFY(SetWindowRgn(window,
                            region,
                            false));
    }
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
        if (WM_SIZE == message)
        {
            OnSize(window, wparam);
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

    CreateWindow(wc.lpszClassName,
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

    auto message = MSG {};

    while (GetMessage(&message, nullptr, 0, 0))
    {
        DispatchMessage(&message);
    }
}
