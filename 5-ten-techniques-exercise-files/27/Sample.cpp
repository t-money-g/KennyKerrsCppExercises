#include "Precompiled.h"

using namespace std;
using namespace KennyKerr;

template <SQLSMALLINT T>
struct sql_traits
{
    using pointer = SQLHANDLE;

    static auto invalid() throw() -> pointer
    {
        return nullptr;
    }

    static auto close(pointer value) throw() -> void
    {
        VERIFY_(SQL_SUCCESS, SQLFreeHandle(T, value));
    }
};

template <SQLSMALLINT T>
using sql_handle = unique_handle<sql_traits<T>>;

using environment = sql_handle<SQL_HANDLE_ENV>;
using connection  = sql_handle<SQL_HANDLE_DBC>;
using statement   = sql_handle<SQL_HANDLE_STMT>;
using descriptor  = sql_handle<SQL_HANDLE_DESC>;

struct sql_diagnostic_info
{
    SQLINTEGER native_error;
    wstring state;
    wstring message;

    sql_diagnostic_info(long n,
                        wstring const & s,
                        wstring const & m) :
        native_error { n },
        state { s },
        message { m }
    {}
};

struct sql_exception
{
    SQLRETURN error;
    SQLSMALLINT handle_type;
    vector<sql_diagnostic_info> records;

    sql_exception(SQLRETURN const result,
                  SQLSMALLINT const type,
                  SQLHANDLE handle) :
        error { result },
        handle_type { type }
    {
        auto native_error = long {};
        wchar_t state[6];
        wchar_t message[1024];
        
        auto record = short {};

        while (SQL_SUCCESS == SQLGetDiagRec(type,
                                            handle,
                                            ++record,
                                            state,
                                            &native_error,
                                            message,
                                            _countof(message),
                                            nullptr))
        {
            records.emplace_back(native_error, state, message);
        }
    }
};

auto sql_check(SQLRETURN const result,
               SQLSMALLINT const type,
               SQLHANDLE handle) -> void
{
    ASSERT(result != SQL_INVALID_HANDLE);

#ifdef _DEBUG
    if (result == SQL_SUCCESS_WITH_INFO)
    {
        auto e = sql_exception { result, type, handle };

        for (auto const & r : e.records)
        {
            TRACE(L"%d %s %s\n",
                  r.native_error,
                  r.state.c_str(),
                  r.message.c_str());
        }
    }
#endif

    if (result != SQL_SUCCESS && result != SQL_SUCCESS_WITH_INFO)
    {
        auto e = sql_exception { result, type, handle };

#ifdef _DEBUG
        for (auto const & r : e.records)
        {
            TRACE(L"%d %s %s\n",
                  r.native_error,
                  r.state.c_str(),
                  r.message.c_str());
        }
#endif

        throw e;
    }
}

auto main() -> int
{
    auto raw = SQLHANDLE {};

    VERIFY_(SQL_SUCCESS, SQLAllocHandle(SQL_HANDLE_ENV,
                                        nullptr,
                                        &raw));

    auto h = environment { raw };
}
