﻿#include "Precompiled.h"

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

struct key_traits
{
    using pointer = BCRYPT_KEY_HANDLE;

    static auto invalid() throw() -> pointer
    {
        return nullptr;
    }

    static auto close(pointer value) throw() -> void
    {
        VERIFY_(ERROR_SUCCESS, BCryptDestroyKey(value));
    }
};

using key = unique_handle<key_traits>;

auto create_key(provider const & p,
                void const * secret,
                unsigned size) -> key
{
    auto k = key {};

    check(BCryptGenerateSymmetricKey(p.get(),
                                     k.get_address_of(),
                                     nullptr,
                                     0,
                                     static_cast<PUCHAR>(const_cast<void *>(secret)),
                                     size,
                                     0));

    return k;
}

auto encrypt(key const & k,
             void const * plaintext,
             unsigned plaintext_size,
             void * ciphertext,
             unsigned ciphertext_size,
             unsigned flags) -> unsigned
{
    auto bytesCopied = ULONG {};

    check(BCryptEncrypt(k.get(),
                        static_cast<PUCHAR>(const_cast<void *>(plaintext)),
                        plaintext_size,
                        nullptr,
                        nullptr,
                        0,
                        static_cast<PUCHAR>(ciphertext),
                        ciphertext_size,
                        &bytesCopied,
                        flags));

    return bytesCopied;
}

auto decrypt(key const & k,
             void const * ciphertext,
             unsigned ciphertext_size,
             void * plaintext,
             unsigned plaintext_size,
             unsigned flags) -> unsigned
{
    auto bytesCopied = ULONG {};

    check(BCryptDecrypt(k.get(),
                        static_cast<PUCHAR>(const_cast<void *>(ciphertext)),
                        ciphertext_size,
                        nullptr,
                        nullptr,
                        0,
                        static_cast<PUCHAR>(plaintext),
                        plaintext_size,
                        &bytesCopied,
                        flags));

    return bytesCopied;
}

auto create_shared_secret(std::string const & secret) -> std::vector<BYTE>
{
    auto p = open_provider(BCRYPT_SHA256_ALGORITHM);

    auto h = create_hash(p);

    combine(h, secret.c_str(), secret.size());

    auto size = unsigned {};

    get_property(h.get(), BCRYPT_HASH_LENGTH, size);

    auto value = std::vector<BYTE> (size);

    get_value(h, &value[0], size);

    return value;
}

auto encrypt_message(std::vector<BYTE> const & shared,
                     std::string const & plaintext) -> std::vector<BYTE>
{
    auto p = open_provider(BCRYPT_3DES_ALGORITHM);

    auto k = create_key(p,
                        &shared[0],
                        shared.size());

    auto size = encrypt(k,
                        plaintext.c_str(),
                        plaintext.size(),
                        nullptr,
                        0,
                        BCRYPT_BLOCK_PADDING);

    std::vector<BYTE> ciphertext (size);

    encrypt(k,
            plaintext.c_str(),
            plaintext.size(),
            &ciphertext[0],
            size,
            BCRYPT_BLOCK_PADDING);

    return ciphertext;
}

auto decrypt_message(std::vector<BYTE> const & shared,
                     std::vector<BYTE> const & ciphertext) -> std::string
{
    auto p = open_provider(BCRYPT_3DES_ALGORITHM);

    auto k = create_key(p,
                        &shared[0],
                        shared.size());

    auto size = decrypt(k,
                        &ciphertext[0],
                        ciphertext.size(),
                        nullptr,
                        0,
                        BCRYPT_BLOCK_PADDING);

    auto plaintext = std::string (size, '\0');

    decrypt(k,
            &ciphertext[0],
            ciphertext.size(),
            &plaintext[0],
            size,
            BCRYPT_BLOCK_PADDING);

    plaintext.resize(strlen(plaintext.c_str()));

    return plaintext;
}

auto main() -> int
{
    auto shared = create_shared_secret("Some shared secret");

    auto ciphertext = encrypt_message(shared, "Secure message");

    auto plaintext = decrypt_message(shared, ciphertext);

    TRACE(L"%S\n", plaintext.c_str());
}
