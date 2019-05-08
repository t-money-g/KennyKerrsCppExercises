#include "Precompiled.h"

using namespace std;
using namespace std::chrono;
using namespace KennyKerr;

auto time_now() -> high_resolution_clock::time_point
{
    return high_resolution_clock::now();
}

auto time_elapsed(high_resolution_clock::time_point const & start) -> float
{
    return duration_cast<duration<float>>(time_now() - start).count();
}

class text_file
{
    char const * m_begin {};
    char const * m_end {};

    auto unmap() throw() -> void
    {
        if (m_begin)
        {
            VERIFY(UnmapViewOfFile(m_begin));
        }
    }

public:

    text_file(text_file const &) = delete;
    auto operator=(text_file const &) -> text_file & = delete;

    explicit text_file(wchar_t const * filename) throw()
    {
        auto file = invalid_handle
        {
            CreateFile(filename,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       nullptr,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       nullptr)
        };

        if (!file) return;

        auto map = null_handle
        {
            CreateFileMapping(file.get(),
                              nullptr,
                              PAGE_READONLY,
                              0, 0,
                              nullptr)
        };

        if (!map) return;

        auto size = LARGE_INTEGER {};

        if (!GetFileSizeEx(file.get(), &size)) return;

        m_begin = static_cast<char const *>(MapViewOfFile(map.get(),
                                                          FILE_MAP_READ,
                                                          0, 0,
                                                          0));

        if (!m_begin) return;

        m_end = m_begin + size.QuadPart;
    }

    ~text_file()
    {
        unmap();
    }

    text_file(text_file && other) throw() :
        m_begin(other.m_begin),
        m_end(other.m_end)
    {
        other.m_begin = nullptr;
        other.m_end = nullptr;
    }

    auto operator=(text_file && other) throw() -> text_file &
    {
        if (this != &other)
        {
            unmap();

            m_begin = other.m_begin;
            m_end = other.m_end;

            other.m_begin = nullptr;
            other.m_end = nullptr;
        }

        return *this;
    }

    explicit operator bool() const throw()
    {
        return m_begin != nullptr;
    }

    auto begin() const throw() -> char const *
    {
        return m_begin;
    }

    auto end() const throw() -> char const *
    {
        return m_end;
    }
};

auto begin(text_file const & file) throw() -> char const *
{
    return file.begin();
}

auto end(text_file const & file) throw() -> char const *
{
    return file.end();
}

auto wmain(int argc, wchar_t ** argv) -> int
{
    auto const start = time_now();
    auto const rx = regex { "\\w+" };

    auto result = map<string, unsigned> {};

    for (int i = 1; i != argc; ++i)
    {
        auto f = text_file { argv[i] };

        if (!f) continue;

        auto l = string {};

        for (auto it = cregex_token_iterator { begin(f), end(f), rx };
                it != cregex_token_iterator {};
                ++it)
        {
            ++result[*it];
        }
    }

    //for (auto const & w: result)
    //{
    //    cout << w.first << " : " << w.second << endl;
    //}

    cout << endl << "Words: " << result.size() 
         << " Seconds: " << time_elapsed(start) << endl;
}
