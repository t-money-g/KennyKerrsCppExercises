#include "Precompiled.h"

using namespace std;
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

    while (S_OK == (hr = reader->Read(&type)))
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

auto get_value(ComPtr<IXmlReader> const & reader) -> wstring
{
    wchar_t const * value {};
    unsigned size {};

    check(reader->GetValue(&value, &size));

    return wstring { value, value + size };
}

using Links = vector<pair<wstring, wstring>>;

auto get_links(ComPtr<IXmlReader> const & reader) -> Links
{
    wchar_t const Item [] = L"item";
    wchar_t const Title [] = L"title";
    wchar_t const Link [] = L"link";

    auto links = Links {};

    while (S_OK == find_element(reader, Item))
    {
        check(find_element(reader, Title));

        auto type = XmlNodeType {};
        check(reader->Read(&type));
        ASSERT(type == XmlNodeType_Text);

        auto title = get_value(reader);

        check(find_element(reader, Link));

        check(reader->Read(&type));
        ASSERT(type == XmlNodeType_Text);

        auto link = get_value(reader);

        links.emplace_back(title, link);
    }

    return links;
}

auto main() -> int
{
    auto reader = download_reader(L"http://kennykerr.ca/feed/");

    auto links = get_links(reader);

    for (auto const & a : links)
    {
        wprintf(L"%s\n%s\n\n",
                a.first.c_str(),
                a.second.c_str());
    }
}
