#include "Precompiled.h"

using namespace std;

auto main(int argc, char ** argv) -> int
{
    for (int i = 1; i != argc; ++i)
    {
        auto f = ifstream { argv[i] };

        if (!f) continue;

        auto w = string {};

        while (f >> w) cout << w << endl;
    }
}
