#include "Precompiled.h"

auto __stdcall wWinMain(HINSTANCE module, HINSTANCE, PWSTR, int) -> int
{
    auto wc = WNDCLASS {};
    wc.lpszClassName = L"window";
    wc.hInstance = module;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(255, 0, 0));

    wc.lpfnWndProc = [](HWND window,
                        UINT message,
                        WPARAM wparam,
                        LPARAM lparam) -> LRESULT
    {
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
                                      RGB(255, 0, 0),
                                      0,
                                      LWA_COLORKEY));

    auto message = MSG {};

    while (GetMessage(&message, nullptr, 0, 0))
    {
        DispatchMessage(&message);
    }
}
