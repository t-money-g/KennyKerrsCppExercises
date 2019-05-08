#include "Precompiled.h"

auto main() -> int
{
    auto dc = GetDC(nullptr);

    auto const x = GetDeviceCaps(dc, LOGPIXELSX);
    auto const y = GetDeviceCaps(dc, LOGPIXELSY);

    ReleaseDC(nullptr, dc);

    printf("System DPI x=%ddpi y=%ddpi (%.f%%)\n",
           x,
           y,
           x / .96f);
}

