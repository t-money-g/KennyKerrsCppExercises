#include "Precompiled.h"

using namespace KennyKerr;
using namespace Microsoft::WRL;

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

struct hash_traits
{
    using pointer = BCRYPT_ALG_HANDLE;

    static auto invalid() throw() -> pointer
    {
        return nullptr;
    }

    static auto close(pointer value) throw() -> void
    {
        VERIFY_(ERROR_SUCCESS, BCryptDestroyHash(value));
    }
};

using hash = unique_handle<hash_traits>;

auto create_hash(provider const & p) -> hash
{
    auto h = hash {};

    check(BCryptCreateHash(p.get(),
                           h.get_address_of(),
                           nullptr,
                           0,
                           nullptr,
                           0,
                           0));

    return h;
}

auto combine(hash const & h,
             void const * buffer,
             unsigned size) -> void
{
    check(BCryptHashData(h.get(),
                         static_cast<PUCHAR>(const_cast<void *>(buffer)),
                         size,
                         0));
}

template <typename T>
auto get_property(BCRYPT_HANDLE handle,
                  wchar_t const * name,
                  T & value) -> void
{
    auto bytesCopied = ULONG {};

    check(BCryptGetProperty(handle,
                            name,
                            reinterpret_cast<PUCHAR>(&value),
                            sizeof(T),
                            &bytesCopied,
                            0));
}

auto get_value(hash const & h,
               void * buffer,
               unsigned size) -> void
{
    check(BCryptFinishHash(h.get(),
                           static_cast<PUCHAR>(buffer),
                           size,
                           0));
}

auto main() -> int
{
    auto p = open_provider(BCRYPT_SHA512_ALGORITHM);

    auto h = create_hash(p);

    auto file = ComPtr<IStream> {};

    VERIFY_(S_OK, SHCreateStreamOnFileEx(L"C:\\Windows\\Explorer.exe",
                                         0,
                                         0,
                                         false,
                                         nullptr,
                                         file.GetAddressOf()));

    BYTE buffer[4096];
    auto size = ULONG {};

    while (SUCCEEDED(file->Read(buffer,
                                sizeof(buffer),
                                &size)) && size)
    {
        combine(h, buffer, size);
    }

    get_property(h.get(),
                 BCRYPT_HASH_LENGTH,
                 size);

    auto value = std::vector<BYTE>(size);
    get_value(h, &value[0], size);

    SecureZeroMemory(&value[0], size);
}
