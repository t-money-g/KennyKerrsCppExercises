#include "Precompiled.h"

auto main() -> int
{
    VERIFY_(S_OK, URLDownloadToFile(nullptr,
                                    L"http://example.org/",
                                    L"C:\\Sample\\page.html",
                                    0, 
                                    nullptr));
}
