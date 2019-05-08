#include "Precompiled.h"

struct data
{
    wchar_t path[10];
    wchar_t overflow[16];
};

auto main() -> int
{
    auto d = data {};
    wcscpy_s(d.path, L"C:\\");
    wcscpy_s(d.overflow, L"Buffer overflow");

    VERIFY(PathAppend(d.path, L"PathTooLong"));

    wprintf(L"[");

    for (auto c : d.path) wprintf(L"%c", (c ? c : L'^'));

    wprintf(L"]\n[");

    for (auto c : d.overflow) wprintf(L"%c", (c ? c : L'^'));

    wprintf(L"]\n");
}
