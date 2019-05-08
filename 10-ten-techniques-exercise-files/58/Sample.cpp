#include "Precompiled.h"

using namespace KennyKerr;

struct provider_traits
{
    using pointer = BCRYPT_ALG_HANDLE;

    static auto invalid() throw() -> pointer
    {
        return nullptr;
    }

    static auto close(pointer value) throw() -> void
    {
        VERIFY_(ERROR_SUCCESS, BCryptCloseAlgorithmProvider(value, 0));
    }
};

using provider = unique_handle<provider_traits>;

struct status_exception
{
    NTSTATUS code;

    status_exception(NTSTATUS result) :
        code { result }
    {}
};

auto check(NTSTATUS const status) -> void
{
    if (ERROR_SUCCESS != status)
    {
        throw status_exception { status };
    }
}

auto open_provider(wchar_t const * algorithm) -> provider
{
    auto p = provider {};

    check(BCryptOpenAlgorithmProvider(p.get_address_of(),
                                      algorithm,
                                      nullptr,
                                      0));

    return p;
}

auto random(provider const & p,
            void * buffer,
            unsigned size) -> void
{
    check(BCryptGenRandom(p.get(),
                          static_cast<PUCHAR>(buffer),
                          size,
                          0));
}

template <typename T, unsigned Count>
auto random(provider const & p,
            T (&buffer)[Count]) -> void
{
    random(p,
           buffer,
           sizeof(T) * Count);
}

template <typename T>
auto random(provider const & p,
            T & buffer) -> void
{
    random(p,
           &buffer,
           sizeof(T));
}

auto main() -> int
{
    auto p = open_provider(BCRYPT_RNG_ALGORITHM);

    unsigned buffer[10] {};
    random(p, buffer);

    auto i = int {};
    random(p, i);

    GUID g;
    random(p, g);
}
