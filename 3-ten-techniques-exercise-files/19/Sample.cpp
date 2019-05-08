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

template <unsigned Count>
auto find_element(ComPtr<IXmlReader> const & reader,
                  wchar_t const (&name)[Count]) throw() -> HRESULT
{
    auto hr = HRESULT {};
    auto type = XmlNodeType {};

    while (S_OK == (hr = reader->Reader(&type)))
    {
        if (XmlNodeType_Element == type)
        {
            wchar_t const * value {};
            auto size = unsigned {};

            if (S_OK != (hr = reader->GetLocalName(&value, &size)))
            {
                break;
            }

            if (Count - 1 == size && 0 == wcscmp(name, value))
            {
                break; // found!
            }
        }
    }

    return hr;
}

auto main() -> int
{
    auto reader = download_reader(L"http://kennykerr.ca/feed/");

    auto type = XmlNodeType {};
    auto indent = unsigned {};

    while (S_OK == reader->Read(&type))
    {
        wchar_t const * name {};

        if (XmlNodeType_Element == type)
        {
            check(reader->GetLocalName(&name, nullptr));

            for (unsigned i = 0; i != indent; ++i) printf(" ");

            if (!reader->IsEmptyElement())
            {
                printf("<%S>\n", name);
                ++indent;
            }
            else
            {
                printf("<%S />\n", name);
            }
        }
        else if (XmlNodeType_EndElement == type)
        {
            check(reader->GetLocalName(&name, nullptr));

            --indent;

            for (unsigned i = 0; i != indent; ++i) printf(" ");

            printf("</%S>\n", name);
        }
    }
}
