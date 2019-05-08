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

auto get_temp() -> wstring
{
    auto const pathSize = GetTempPath(0, nullptr);

    if (!pathSize) throw_windows_error();

    unsigned const GuidSize = 38;

    auto path = wstring ( pathSize + GuidSize - 1, L' ' );

    if (!GetTempPath(pathSize, &path[0]))
    {
        throw_windows_error();
    }

    auto const result = SHCreateDirectory(nullptr, path.c_str());

    if (result != ERROR_SUCCESS && result != ERROR_ALREADY_EXISTS)
    {
        throw_windows_error(result);
    }

    GUID guid;
    check(CoCreateGuid(&guid));

    VERIFY_(GuidSize + 1, StringFromGUID2(guid,
                                          &path[pathSize - 2],
                                          GuidSize + 1));

    path[pathSize - 2] = L'\\';

    path.resize(pathSize + GuidSize - 3);

    return path;
}

auto temp_stream(wstring const & filename) -> ComPtr<IStream>
{
    auto stream = ComPtr<IStream> {};

    check(SHCreateStreamOnFileEx(filename.c_str(),
                                 STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,
                                 FILE_ATTRIBUTE_NORMAL,
                                 true,
                                 nullptr,
                                 stream.GetAddressOf()));

    return stream;
}

auto write_html(ComPtr<IStream> const & stream,
                Links const & links) -> void
{
    auto writer = ComPtr<IXmlWriter> {};

    check(CreateXmlWriter(__uuidof(writer),
                          reinterpret_cast<void **>(writer.GetAddressOf()),
                          nullptr));

    check(writer->SetProperty(XmlWriterProperty_Indent, true));
    check(writer->SetOutput(stream.Get()));

    check(writer->WriteDocType(L"html", nullptr, nullptr, nullptr));

    check(writer->WriteStartElement(nullptr, L"html", nullptr));

    check(writer->WriteStartElement(nullptr, L"head", nullptr));

    check(writer->WriteStartElement(nullptr, L"meta", nullptr));
    check(writer->WriteAttributeString(nullptr, L"charset", nullptr, L"UTF-8"));
    check(writer->WriteEndElement()); // meta

    check(writer->WriteStartElement(nullptr, L"style", nullptr));
    check(writer->WriteRaw(L"p{font-family:Myriad Pro}"));
    check(writer->WriteEndElement()); // style

    check(writer->WriteEndElement()); // head

    check(writer->WriteStartElement(nullptr, L"body", nullptr));

    for (auto const & e : links)
    {
        check(writer->WriteStartElement(nullptr, L"p", nullptr));

        check(writer->WriteStartElement(nullptr, L"a", nullptr));
        check(writer->WriteAttributeString(nullptr, L"href", nullptr, e.second.c_str()));
        check(writer->WriteChars(e.first.c_str(), e.first.size()));
        check(writer->WriteEndElement()); // a

        check(writer->WriteEndElement()); // p
    }

    check(writer->WriteEndElement()); // body
    check(writer->WriteEndElement()); // html
}

auto main() -> int
{
    auto reader = download_reader(L"http://kennykerr.ca/feed/");

    auto links = get_links(reader);

    auto filename = get_temp();
    filename += L".htm";

    auto stream = temp_stream(filename);

    write_html(stream, links);

    stream.Reset();

    ShellExecute(nullptr,
                 nullptr,
                 filename.c_str(),
                 nullptr,
                 nullptr,
                 SW_SHOWDEFAULT);
}
