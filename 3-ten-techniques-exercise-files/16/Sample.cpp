#include "Precompiled.h"

using namespace Microsoft::WRL;

struct com_exception
{
    HRESULT result;

    explicit com_exception(HRESULT const value) :
        result { value }
    {}
};

auto check(HRESULT const result) -> void
{
    if (result != S_OK)
    {
        throw com_exception { result };
    }
}

auto throw_windows_error(DWORD const error = GetLastError()) -> void
{
    throw com_exception { HRESULT_FROM_WIN32(error) };
}

auto main() -> int
{
}
