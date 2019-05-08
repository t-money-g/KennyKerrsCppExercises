#include "Precompiled.h"

using namespace std;
using namespace KennyKerr;

struct windows_exception
{
    DWORD error;

    windows_exception(DWORD const value = GetLastError()) :
        error { value }
    {}
};

struct winhttp_traits
{
    using pointer = HINTERNET;

    static auto invalid() throw() -> pointer
    {
        return nullptr;
    }

    static auto close(pointer value) throw() -> void
    {
        VERIFY(WinHttpCloseHandle(value));
    }
};

using winhttp = unique_handle<winhttp_traits>;

auto main() -> int
{
    auto session = winhttp
    {
        WinHttpOpen(nullptr,
                    WINHTTP_ACCESS_TYPE_NO_PROXY,
                    nullptr,
                    nullptr,
                    0)
    };

    if (!session) throw windows_exception {};

    auto connection = winhttp
    {
        WinHttpConnect(session.get(),
                       L"localhost",
                       8000,
                       0)
    };

    if (!connection) throw windows_exception {};

    auto request = winhttp
    {
        WinHttpOpenRequest(connection.get(),
                           nullptr,
                           L"/ws",
                           nullptr,
                           nullptr,
                           nullptr,
                           0)
    };

    if (!request) throw windows_exception {};

    if (!WinHttpSetOption(request.get(),
                        WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET,
                        nullptr,
                        0))
    {
        throw windows_exception {};
    }

    if (!WinHttpSendRequest(request.get(),
                            nullptr,
                            0,
                            nullptr,
                            0,
                            0,
                            0))
    {
        throw windows_exception {};
    }

    if (!WinHttpReceiveResponse(request.get(),
                                nullptr))
    {
        throw windows_exception {};
    }

    auto status = DWORD {};
    auto size = DWORD { sizeof(status) };

    if (!WinHttpQueryHeaders(request.get(),
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        WINHTTP_HEADER_NAME_BY_INDEX,
        &status,
        &size,
        nullptr))
    {
        throw windows_exception {};
    }

    if (HTTP_STATUS_SWITCH_PROTOCOLS != status)
    {
        printf("Not websocket server\n");
        return 0;
    }

    auto ws = winhttp
    {
        WinHttpWebSocketCompleteUpgrade(request.get(),
                                        0)
    };

    if (!ws) throw windows_exception {};
}
