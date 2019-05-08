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
using connection_handle = sql_handle<SQL_HANDLE_DBC>;
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

template <typename T>
auto sql_allocate_handle(SQLSMALLINT const type,
                         SQLHANDLE input) -> T
{
    auto h = SQLHANDLE {};

    auto const r = SQLAllocHandle(type,
                                  input,
                                  &h);

    sql_check(r, type, input);

    return T { h };
}

auto create_environment() -> environment
{
    auto e = sql_allocate_handle<environment>(SQL_HANDLE_ENV, nullptr);

    auto const r = SQLSetEnvAttr(e.get(),
                                 SQL_ATTR_ODBC_VERSION,
                                 reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3_80),
                                 SQL_INTEGER);

    sql_check(r, SQL_HANDLE_ENV, e.get());

    return e;
}

auto create_connection(environment const & e) -> connection_handle
{
    return sql_allocate_handle<connection_handle>(SQL_HANDLE_DBC, e.get());
}

auto connect(connection_handle const & c,
             wchar_t const * connection_string) -> void
{
    auto const r = SQLDriverConnect(c.get(),
                                    nullptr,
                                    const_cast<wchar_t *>(connection_string),
                                    SQL_NTS,
                                    nullptr,
                                    0,
                                    nullptr,
                                    SQL_DRIVER_NOPROMPT);

    sql_check(r, SQL_HANDLE_DBC, c.get());
}

class connection
{
    connection_handle m_handle;
    bool m_connected { false };

public:

    connection(environment const & e) :
        m_handle { create_connection(e) }
    {
    }

    auto connect(wchar_t const * connection_string) -> void
    {
        ASSERT(!m_connected);

        ::connect(m_handle, connection_string);

        m_connected = true;
    }

    ~connection()
    {
        if (m_connected)
        {
            VERIFY_(SQL_SUCCESS, SQLDisconnect(m_handle.get()));
        }
    }

    auto create_statement() -> statement
    {
        return sql_allocate_handle<statement>(SQL_HANDLE_STMT,
                                              m_handle.get());
    }
};

auto execute(statement const & s,
             wchar_t const * text) -> void
{
    auto const r = SQLExecDirect(s.get(),
                                 const_cast<wchar_t *>(text),
                                 SQL_NTS);

    sql_check(r, SQL_HANDLE_STMT, s.get());
}

static wchar_t connection_string[] = L"Driver=SQL Server Native Client 11.0;Server=tcp:x4aoy5jfej.database.windows.net,1433;Database=Master;Encrypt=yes;Uid=kenny@x4aoy5jfej;Pwd=OVOdBr53#49Fbkife8vE";

auto main() -> int
{
    auto e = create_environment();

    auto c = connection { e };

    c.connect(connection_string);

    auto s = c.create_statement();

    execute(s, L"create database Chickens");
}
