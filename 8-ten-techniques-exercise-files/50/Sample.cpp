#include "Precompiled.h"

using namespace KennyKerr;
using namespace KennyKerr::Direct2D;

static auto factory = CreateFactory();
static auto target = HwndRenderTarget {};
static auto brush = SolidColorBrush {};
static auto const White = Color { 1.0f, 1.0f, 1.0f };
static auto const Orange = Color { 0.92f, 0.38f, 0.208f };

auto OnPaint(HWND window) -> void
{
    if (!target)
    {
        target = factory.CreateHwndRenderTarget(window);
        brush = target.CreateSolidColorBrush(Orange);
    }

    target.BeginDraw();
    target.Clear(White);

    auto size = target.GetSize();

    auto rect = RectF(100.0f,
                      100.0f,
                      size.Width - 100.0f,
                      size.Height - 100.0f);

    auto rounded = RoundedRect(rect, 100.0f, 100.0f);

    target.DrawRoundedRectangle(rounded, brush, 100.0f);

    if (S_OK != target.EndDraw())
    {
        target.Reset();
    }
}

auto OnSize(LPARAM lparam) -> void
{
    if (target)
    {
        auto newSize = SizeU(LOWORD(lparam), HIWORD(lparam));

        if (S_OK != target.Resize(newSize))
        {
            target.Reset();
        }
    }
}

auto __stdcall wWinMain(HINSTANCE module, HINSTANCE, PWSTR, int) -> int
{
    auto wc = WNDCLASS {};
    wc.lpszClassName = L"window";
    wc.hInstance = module;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW;

    wc.lpfnWndProc = [](HWND window,
                        UINT message,
                        WPARAM wparam,
                        LPARAM lparam) -> LRESULT
    {
        if (WM_PAINT == message)
        {
            OnPaint(window);
            VERIFY(ValidateRect(window, nullptr));
            return 0;
        }

        if (WM_SIZE == message)
        {
            OnSize(lparam);
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
