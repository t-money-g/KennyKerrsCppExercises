#include "Precompiled.h"

using namespace std;

auto main(int argc, char ** argv) -> int
{
    auto result = map<string, unsigned> {};

    for (int i = 1; i != argc; ++i)
    {
        auto f = ifstream { argv[i] };

        if (!f) continue;

        auto w = string {};

        while (f >> w) ++result[w];
    }

    for (auto const & w: result)
    {
        cout << w.first << " : " << w.second << endl;
    }

    cout << endl << "Words: " << result.size() << endl;
}
