#include "Precompiled.h"

using namespace KennyKerr;
using namespace KennyKerr::Direct2D;

struct LayeredWindow
{
    UPDATELAYEREDWINDOWINFO info;
    BLENDFUNCTION blend;
    SIZE size;
    POINT source, destination;

    LayeredWindow(unsigned width, unsigned height) :
        info {},
        blend {},
        size {},
        source {},
        destination {}
    {
        info.cbSize = sizeof(UPDATELAYEREDWINDOWINFO);
        info.pblend = &blend;
        info.psize = &size;
        info.pptSrc = &source;
        info.pptDst = &destination;
        info.dwFlags = ULW_ALPHA;

        blend.SourceConstantAlpha = 255;
        blend.AlphaFormat = AC_SRC_ALPHA;

        size.cx = width;
        size.cy = height;
    }

    auto Update(HWND window, HDC source) -> void
    {
        info.hdcSrc = source;
        VERIFY(UpdateLayeredWindowIndirect(window, &info));
    }
};

static auto factory = CreateFactory();
static auto target = RenderTarget {};
static auto layered = LayeredWindow { 750, 750 };

static auto CreateRenderTarget() -> RenderTarget
{
    auto wicFactory = Wic::CreateFactory();

    auto size = SizeU
    {
        static_cast<unsigned>(layered.size.cx),
        static_cast<unsigned>(layered.size.cy)
    };

    auto bitmap = wicFactory.CreateBitmap(size);

    auto format = PixelFormat
    {
        Dxgi::Format::B8G8R8A8_UNORM,
        AlphaMode::Premultiplied
    };

    auto properties = RenderTargetProperties
    {
        RenderTargetType::Default,
        format
    };

    properties.Usage = RenderTargetUsage::GdiCompatible;

    return factory.CreateWicBitmapRenderTarget(bitmap, properties);
}

auto Render(HWND window) -> void
{
    target.BeginDraw();
    target.Clear();

    auto size = target.GetSize();

    auto translation = D2D1::Matrix3x2F::Translation(size.Width / 2.0f,
                                                     size.Height / 2.0f);

    static auto angle = float {};

    target.SetTransform(D2D1::Matrix3x2F::Rotation(++angle) * translation);

    auto rect = RectF { -200.0f, -200.0f, 200.0f, 200.0f };

    auto orange = Color { 0.92f, 0.38f, 0.208f };

    auto brush = target.CreateSolidColorBrush(orange);

    target.DrawRectangle(rect, brush, 100.0);

    auto interop = target.AsGdiInteropRenderTarget();

    auto dc = interop.GetDC(DcInitializeMode::Copy);

    layered.Update(window, dc);

    interop.ReleaseDC();

    if (S_OK != target.EndDraw())
    {
        target.Reset();
    }
}

auto __stdcall wWinMain(HINSTANCE module, HINSTANCE, PWSTR, int) -> int
{
    auto oldschool = ComInitialize {};

    auto wc = WNDCLASS {};
    wc.hInstance = module;
    wc.lpszClassName = L"window";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    wc.lpfnWndProc = [] (HWND window,
                         UINT message,
                         WPARAM wparam,
                         LPARAM lparam) -> LRESULT
    {
        if (WM_KEYDOWN == message)
        {
            Render(window);
            return 0;
        }

        if (WM_NCHITTEST == message)
        {
            return HTCAPTION;
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
                                 layered.size.cx,
                                 layered.size.cy,
                                 nullptr,
                                 nullptr,
                                 module,
                                 nullptr);

    target = CreateRenderTarget();
    Render(window);

    auto message = MSG {};

    while (GetMessage(&message, nullptr, 0, 0))
    {
        DispatchMessage(&message);
    }

    target.Reset();
}
