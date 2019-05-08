#include "Precompiled.h"

auto main() -> int
{
    wchar_t filename[MAX_PATH];

    VERIFY_(S_OK, URLDownloadToCacheFile(nullptr,
                                         L"http://example.org/",
                                         filename,
                                         _countof(filename),
                                         0, 
                                         nullptr));

    ShellExecute(nullptr,
                 nullptr,
                 filename,
                 nullptr,
                 nullptr,
                 SW_SHOWDEFAULT);
}
