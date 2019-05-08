#include "Precompiled.h"

using namespace Microsoft::WRL;

struct com_exception
{
    HRESULT result;

    explicit com_exception(HRESULT const value) :
        result { value }
    {}
};

auto check(HRESULT const result) -> void
{
    if (result != S_OK)
    {
        throw com_exception { result };
    }
}

auto throw_windows_error(DWORD const error = GetLastError()) -> void
{
    throw com_exception { HRESULT_FROM_WIN32(error) };
}

auto download_reader(wchar_t const * url) -> ComPtr<IXmlReader>
{
    auto stream = ComPtr<IStream> {};

    check(URLOpenBlockingStream(nullptr,
                                url,
                                stream.GetAddressOf(),
                                0,
                                nullptr));

    auto reader = ComPtr<IXmlReader> {};

    check(CreateXmlReader(__uuidof(reader),
                          reinterpret_cast<void **>(reader.GetAddressOf()),
                          nullptr));

    check(reader->SetInput(stream.Get()));

    return reader;
}

auto main() -> int
{
    auto reader = download_reader(L"http://kennykerr.ca/feed/");
}
