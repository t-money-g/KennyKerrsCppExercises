#include "Precompiled.h"

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
using connection = sql_handle<SQL_HANDLE_DBC>;
using statement = sql_handle<SQL_HANDLE_STMT>;
using descriptor = sql_handle<SQL_HANDLE_DESC>;

auto main() -> int
{
    auto raw = SQLHANDLE {};

    VERIFY_(SQL_SUCCESS, SQLAllocHandle(SQL_HANDLE_ENV,
                                        nullptr,
                                        &raw));

    auto h = environment { raw };
}
