#include "Precompiled.h"

using namespace std;

auto main() -> int
{
    auto result = vector<string> {};

    for (auto w = string {}; cin >> w; )
    {
        result.emplace_back(w);
    }

    for (auto const & w : result)
    {
        cout << w << endl;
    }
}
