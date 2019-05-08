#include "Precompiled.h"

using namespace std;
using namespace std::chrono;

auto time_now() -> high_resolution_clock::time_point
{
    return high_resolution_clock::now();
}

auto time_elapsed(high_resolution_clock::time_point const & start) -> float
{
    return duration_cast<duration<float>>(time_now() - start).count();
}

auto main(int argc, char ** argv) -> int
{
    auto const start = time_now();
    auto const rx = regex { "\\w+" };

    auto result = map<string, unsigned> {};

    for (int i = 1; i != argc; ++i)
    {
        auto f = ifstream { argv[i] };

        if (!f) continue;

        auto l = string {};

        while (getline(f, l))
        {
            for (auto it = sregex_token_iterator { begin(l), end(l), rx };
                 it != sregex_token_iterator {};
                 ++it)
            {
                ++result[*it];
            }
        }
    }

    //for (auto const & w: result)
    //{
    //    cout << w.first << " : " << w.second << endl;
    //}

    cout << endl << "Words: " << result.size() 
         << " Seconds: " << time_elapsed(start) << endl;
}
