#include "Precompiled.h"

using namespace std;

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

        //VERIFY(PathAppend(path, s.c_str()));

        check(PathCchAppend(path,
                            _countof(path),
                            s.c_str()));
    }
    catch (path_exception const & e)
    {
        TRACE(L"0x%X\n", e.code);
    }
}
