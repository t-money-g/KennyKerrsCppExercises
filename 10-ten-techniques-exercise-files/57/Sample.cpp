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

auto main() -> int
{
}
