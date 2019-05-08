#include "Precompiled.h"

using namespace std;
using namespace KennyKerr;

struct local_message_traits
{
    using pointer = wchar_t *;

    static auto invalid() throw() -> pointer
    {
        return nullptr;
    }

    static auto close(pointer value) throw() -> void
    {
        VERIFY_(nullptr, LocalFree(value));
    }
};

auto error_message(HRESULT const code) -> wstring
{
    auto local = unique_handle<local_message_traits> {};

    auto const flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                       FORMAT_MESSAGE_FROM_SYSTEM |
                       FORMAT_MESSAGE_IGNORE_INSERTS;

    auto size = FormatMessage(flags,
                              nullptr,
                              code,
                              0,
                              reinterpret_cast<wchar_t *>(local.get_address_of()),
                              0, 
                              nullptr);

    while (size && iswspace(*(local.get() + size - 1)))
    {
        --size;
    }

    if (0 == size) return L"I blame the weather!";

    return wstring { local.get(), local.get() + size };
}

struct path_exception
{
    HRESULT code;

    explicit path_exception(HRESULT const result) :
        code { result }
    {}
};

auto check(HRESULT const result) -> void
{
    if (result != S_OK)
    {
        throw path_exception { result };
    }
}

auto main() -> int
{
    try
    {
        wchar_t path [MAX_PATH] {};

        auto s = wstring { L"C:\\Path" };
        s.resize(MAX_PATH, L'o');

        check(PathCchAppend(path,
                            _countof(path),
                            s.c_str()));
    }
    catch (path_exception const & e)
    {
        TRACE(L"0x%X [%s]\n", e.code, error_message(e.code).c_str());
    }
}
