#include "Precompiled.h"

using namespace Microsoft::WRL;

static auto GetPosition(HANDLE console) -> COORD
{
    CONSOLE_SCREEN_BUFFER_INFO info;

    VERIFY(GetConsoleScreenBufferInfo(console, &info));

    return info.dwCursorPosition;
}

static auto ShowCursor(HANDLE console,
                       bool const visible = true) -> void
{
    CONSOLE_CURSOR_INFO info;

    VERIFY(GetConsoleCursorInfo(console, &info));

    info.bVisible = visible;

    VERIFY(SetConsoleCursorInfo(console, &info));
}

struct ProgressCallback :
    RuntimeClass<RuntimeClassFlags<ClassicCom>,
                 IBindStatusCallback>
{
    HANDLE m_console;
    COORD m_position;

    ProgressCallback(HANDLE console,
                     COORD position) :
        m_console(console),
        m_position(position)
    {}

    auto __stdcall OnProgress(ULONG progress,
                              ULONG progressMax,
                              ULONG,
                              LPCWSTR) -> HRESULT override
    {
        if (0 < progress && progress <= progressMax)
        {
            float percentF = progress * 100.0f / progressMax;
            unsigned percentU = min(100U, static_cast<unsigned>(percentF));

            VERIFY(SetConsoleCursorPosition(m_console,
                                            m_position));

            printf("%3d%%\n", percentU);
        }

        return S_OK;
    }

    auto __stdcall OnStartBinding(DWORD, IBinding *) -> HRESULT override
    {
        return E_NOTIMPL;
    }

    auto __stdcall GetPriority(LONG *) -> HRESULT override
    {
        return E_NOTIMPL;
    }

    auto __stdcall OnLowResource(DWORD) -> HRESULT override
    {
        return E_NOTIMPL;
    }

    auto __stdcall OnStopBinding(HRESULT, LPCWSTR) -> HRESULT override
    {
        return E_NOTIMPL;
    }

    auto __stdcall GetBindInfo(DWORD *, BINDINFO *) -> HRESULT override
    {
        return E_NOTIMPL;
    }

    auto __stdcall OnDataAvailable(DWORD, DWORD, FORMATETC *, STGMEDIUM *) -> HRESULT override
    {
        return E_NOTIMPL;
    }
    
    auto __stdcall OnObjectAvailable(REFIID, IUnknown *) -> HRESULT override
    {
        return E_NOTIMPL;
    }
};

auto main() -> int
{
    wchar_t filename[MAX_PATH];

    auto console = GetStdHandle(STD_OUTPUT_HANDLE);
    ShowCursor(console, false);

    auto callback = ProgressCallback
    {
        console,
        GetPosition(console)
    };

    VERIFY_(S_OK, URLDownloadToCacheFile(nullptr,
                                         L"http://live.sysinternals.com/procexp.exe",
                                         filename,
                                         _countof(filename),
                                         0, 
                                         &callback));

    ShowCursor(console);

    ShellExecute(nullptr,
                 nullptr,
                 filename,
                 nullptr,
                 nullptr,
                 SW_SHOWDEFAULT);
}
