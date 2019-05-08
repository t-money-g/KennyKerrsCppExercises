#include "Precompiled.h"

using namespace std;
using namespace KennyKerr;
using namespace KennyKerr::Direct2D;
using namespace KennyKerr::DirectWrite;

static auto CreateTextFormat() -> TextFormat
{
    auto factory = DirectWrite::CreateFactory();

    auto format = factory.CreateTextFormat(L"Myriad Pro",
                                           50.0f);

    format.SetWordWrapping(WordWrapping::NoWrap);
    format.SetIncrementalTabStop(75.0f);

    return format;
}

template <typename... Args>
auto append(wstring & s, wchar_t const * f, Args... args) -> void
{
    auto const size = s.size();
    s.resize(size + 255);

    auto const count = swprintf_s(&s[size], 256, f, args...);

    s.resize(size + count);
}

auto GetText(HWND window) -> wstring
{
    auto text = wstring {};

    auto monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);

    auto args = make_pair(&text, monitor);

    EnumDisplayMonitors(nullptr,
                        nullptr,
    [] (HMONITOR monitor, HDC, RECT *, LPARAM args)
    {
        auto & data = *reinterpret_cast<pair<wstring *, HMONITOR> *>(args);

        auto x = unsigned {};
        auto y = unsigned {};
        VERIFY_(S_OK, GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &x, &y));

        if (monitor == data.second)
        {
            data.first->append(L"😉");
        }

        append(*data.first, L"\tMonitor: x=%ddpi y=%ddpi (%.f%%)\n",
               x, y,
               x / .96f);

        return TRUE;
    },
    reinterpret_cast<LPARAM>(&args));

    auto rect = RECT {};
    GetWindowRect(window, &rect);

    append(text, L"\tSize: %d x %d\n",
           rect.right - rect.left,
           rect.bottom - rect.top);

    return text;
}

static auto factory = Direct2D::CreateFactory();
static auto target = HwndRenderTarget {};
static auto brush = SolidColorBrush {};
static auto format = CreateTextFormat();
static auto const White = Color { 1.0f, 1.0f, 1.0f };

auto OnPaint(HWND window) -> void
{
    if (!target)
    {
        target = factory.CreateHwndRenderTarget(window);
        brush = target.CreateSolidColorBrush(Color {});

        auto monitor = MonitorFromWindow(window,
                                         MONITOR_DEFAULTTONEAREST);

        auto x = unsigned {};
        auto y = unsigned {};

        VERIFY_(S_OK, GetDpiForMonitor(monitor,
                                       MDT_EFFECTIVE_DPI,
                                       &x, &y));

        target.SetDpi(static_cast<float>(x),
                      static_cast<float>(y));
    }

    target.BeginDraw();
    target.Clear(White);

    auto size = target.GetSize();

    auto rect = RectF { 50.0f, 50.0f, size.Width - 50.0f, size.Height - 50.0f };

    auto text = GetText(window);

    target.DrawText(text.c_str(),
                    static_cast<unsigned>(text.size()),
                    format,
                    rect,
                    brush,
                    DrawTextOptions::EnableColorFont);

    if (S_OK != target.EndDraw())
    {
        target.Reset();
    }
}

auto OnDpiChanged(HWND window, WPARAM wparam, LPARAM lparam) -> void
{
    if (target)
    {
        auto x = HIWORD(wparam);
        auto y = LOWORD(wparam);

        target.SetDpi(x, y);

        auto rect = *reinterpret_cast<RECT *>(lparam);

        VERIFY(SetWindowPos(window,
                            0,
                            rect.left,
                            rect.top,
                            rect.right - rect.left,
                            rect.bottom - rect.top,
                            SWP_NOACTIVATE | SWP_NOZORDER));
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
    wc.hInstance = module;
    wc.lpszClassName = L"window";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.style = CS_HREDRAW | CS_VREDRAW;

    wc.lpfnWndProc = [] (HWND window, UINT message, WPARAM wparam, LPARAM lparam) -> LRESULT
    {
        if (WM_PAINT == message)
        {
            OnPaint(window);
            VERIFY(ValidateRect(window, nullptr));
            return 0;
        }

        if (WM_DPICHANGED == message)
        {
            OnDpiChanged(window, wparam, lparam);
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

    CreateWindow(wc.lpszClassName, L"Sample",
                 WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                 nullptr, nullptr, module, nullptr);

    auto message = MSG {};

    while (GetMessage(&message, nullptr, 0, 0))
    {
        DispatchMessage(&message);
    }
}
