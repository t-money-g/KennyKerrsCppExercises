#include "Precompiled.h"

using namespace Microsoft::WRL;

struct ProgressCallback :
    RuntimeClass<RuntimeClassFlags<ClassicCom>,
                 IBindStatusCallback>
{
    auto __stdcall OnProgress(ULONG progress,
                              ULONG progressMax,
                              ULONG statusCode,
                              LPCWSTR statusText) -> HRESULT override
    {
        auto const statusEnum = static_cast<BINDSTATUS>(statusCode);

        printf("%2d> %S\n(%d/%d)\n\n",
               statusEnum,
               statusText,
               progress,
               progressMax);

        return S_OK;
    }

    auto __stdcall OnStartBinding(DWORD, IBinding *) -> HRESULT override
    {
        return E_NOTIMPL;
    }

    auto __stdcall GetPriority(LONG *) -> HRESULT override
    {
        return E_NOTIMPL;
    }

    auto __stdcall OnLowResource(DWORD) -> HRESULT override
    {
        return E_NOTIMPL;
    }

    auto __stdcall OnStopBinding(HRESULT, LPCWSTR) -> HRESULT override
    {
        return E_NOTIMPL;
    }

    auto __stdcall GetBindInfo(DWORD *, BINDINFO *) -> HRESULT override
    {
        return E_NOTIMPL;
    }

    auto __stdcall OnDataAvailable(DWORD, DWORD, FORMATETC *, STGMEDIUM *) -> HRESULT override
    {
        return E_NOTIMPL;
    }
    
    auto __stdcall OnObjectAvailable(REFIID, IUnknown *) -> HRESULT override
    {
        return E_NOTIMPL;
    }
};

auto main() -> int
{
    wchar_t filename[MAX_PATH];

    auto callback = ProgressCallback {};

    VERIFY_(S_OK, URLDownloadToCacheFile(nullptr,
                                         L"http://example.org/",
                                         filename,
                                         _countof(filename),
                                         0, 
                                         &callback));
}
