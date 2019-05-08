#include "Precompiled.h"

auto main() -> int
{
    VERIFY_(S_OK, SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE));

    auto dc = GetDC(nullptr);

    auto const x = GetDeviceCaps(dc, LOGPIXELSX);
    auto const y = GetDeviceCaps(dc, LOGPIXELSY);

    ReleaseDC(nullptr, dc);

    printf("System DPI x=%ddpi y=%ddpi (%.f%%)\n",
           x,
           y,
           x / .96f);

    auto awareness = PROCESS_DPI_AWARENESS {};

    VERIFY_(S_OK, GetProcessDpiAwareness(nullptr,
                                         &awareness));

    printf("DPI awareness: ");

    if (awareness == PROCESS_SYSTEM_DPI_AWARE)
    {
        printf("System Aware\n");
    }
    else if (awareness == PROCESS_PER_MONITOR_DPI_AWARE)
    {
        printf("Per-Monitor Aware\n");
    }
    else
    {
        printf("Unaware\n");
    }
}

