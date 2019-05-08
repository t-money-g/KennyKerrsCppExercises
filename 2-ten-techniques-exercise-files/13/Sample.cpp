#include "Precompiled.h"

using namespace Microsoft::WRL;

struct StreamInfo : STATSTG
{
    StreamInfo() : STATSTG {}
    {
    }

    ~StreamInfo()
    {
        if (pwcsName)
        {
            CoTaskMemFree(pwcsName);
        }
    }
};

auto main() -> int
{
    auto stream = ComPtr<IStream> {};

    VERIFY_(S_OK, URLOpenBlockingStream(nullptr,
                                        L"http://example.org/",
                                        stream.GetAddressOf(),
                                        0,
                                        nullptr));

    auto info = StreamInfo {};

    VERIFY_(S_OK, stream->Stat(&info, STATFLAG_DEFAULT));

    ShellExecute(nullptr,
                 nullptr,
                 info.pwcsName,
                 nullptr,
                 nullptr,
                 SW_SHOWDEFAULT);
}
