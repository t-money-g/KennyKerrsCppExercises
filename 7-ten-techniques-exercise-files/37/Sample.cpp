#include "Precompiled.h"

using namespace std;

auto main() -> int
{
    wchar_t path [MAX_PATH] {};

    auto s = wstring { L"C:\\Path" };

    s.resize(MAX_PATH, L'o');

    VERIFY(PathAppend(path, s.c_str()));
}
