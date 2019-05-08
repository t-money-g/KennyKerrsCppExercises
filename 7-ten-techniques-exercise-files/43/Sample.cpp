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

namespace path
{
    namespace details
    {
        auto trim(wstring & path) -> void
        {
            path.resize(wcslen(path.c_str()));
        }

        auto append(wstring & path,
                    wchar_t const * more,
                    size_t const size) -> void
        {
            // \\?\path\more

            path.resize(path.size() + 5 + size);

            check(PathCchAppendEx(&path[0],
                                  path.size() + 1,
                                  more,
                                  PATHCCH_ALLOW_LONG_PATHS));

            trim(path);
        }
    }

    auto append(wstring & path,
                wchar_t const * more) -> void
    {
        details::append(path,
                        more,
                        wcslen(more));
    }
    
    auto append(wstring & path,
                wstring const & more) -> void
    {
        details::append(path,
                        more.c_str(),
                        more.size());
    }

    auto get_short(wchar_t const * path) -> wstring
    {
        auto const size = GetShortPathName(path, nullptr, 0);

        if (!size) throw path_exception { HRESULT_FROM_WIN32(GetLastError()) };

        auto result = wstring ( size - 1, L' ' );

        if (size - 1 != GetShortPathName(path,
                                         &result[0],
                                         static_cast<DWORD>(result.size() + 1)))
        {
            throw path_exception { HRESULT_FROM_WIN32(GetLastError()) };
        }

        return result;
    }

    auto get_short(wstring const & path) -> wstring
    {
        return get_short(path.c_str());
    }

    auto get_long(wchar_t const * path) -> wstring
    {
        auto const size = GetLongPathName(path, nullptr, 0);

        if (!size) throw path_exception { HRESULT_FROM_WIN32(GetLastError()) };

        auto result = wstring ( size - 1, L' ' );

        if (size - 1 != GetLongPathName(path,
                                         &result[0],
                                         static_cast<DWORD>(result.size() + 1)))
        {
            throw path_exception { HRESULT_FROM_WIN32(GetLastError()) };
        }

        return result;
    }

    auto get_long(wstring const & path) -> wstring
    {
        return get_long(path.c_str());
    }

    auto remove_prefix(wstring & path) -> bool
    {
        auto const result = PathCchStripPrefix(&path[0],
                                               path.size() + 1);

        if (S_OK == result)
        {
            details::trim(path);
            return true;
        }

        if (S_FALSE == result)
        {
            return false;
        }

        throw path_exception { result };
    }

    auto remove_filename(wstring & path) -> bool
    {
        auto const result = PathCchRemoveFileSpec(&path[0],
                                                  path.size() + 1);

        if (S_OK == result)
        {
            details::trim(path);
            return true;
        }

        if (S_FALSE == result)
        {
            return false;
        }

        throw path_exception { result };
    }
}

auto main() -> int
{
    try
    {
        auto p = wstring { LR"(C:\Sample\Chickens\Hens\Eggs\Candling.txt)" };
        path::remove_filename(p);

        for (auto c = L'A'; c <= L'Z'; ++c)
        {
            path::append(p, wstring (20, c));
        }

        auto const result = SHCreateDirectoryEx(nullptr, p.c_str(), nullptr);

        if (ERROR_SUCCESS != result && ERROR_ALREADY_EXISTS != result)
        {
            throw path_exception { HRESULT_FROM_WIN32(result) };
        }
    }
    catch (path_exception const & e)
    {
        TRACE(L"0x%X [%s]\n", e.code, error_message(e.code).c_str());
    }
}
