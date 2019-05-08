#include "Precompiled.h"

using namespace std;
using namespace KennyKerr;

struct connection_handle_traits
{
    using pointer = sqlite3 *;

    static auto invalid() throw() -> pointer
    {
        return nullptr;
    }

    static auto close(pointer value) throw() -> void
    {
        VERIFY_(SQLITE_OK, sqlite3_close(value));
    }
};

using connection_handle = unique_handle<connection_handle_traits>;

struct statement_handle_traits
{
    using pointer = sqlite3_stmt *;

    static auto invalid() throw() -> pointer
    {
        return nullptr;
    }

    static auto close(pointer value) throw() -> void
    {
        VERIFY_(SQLITE_OK, sqlite3_finalize(value));
    }
};

using statement_handle = unique_handle<statement_handle_traits>;

struct sql_exception
{
    int code;
    string message;

    sql_exception(int result, char const * text) :
        code { result },
        message { text }
    {}
};

struct connection
{
    connection_handle handle;

    auto open(char const * filename) -> void
    {
        auto local = connection_handle {};

        auto const result = sqlite3_open(filename,
                                         local.get_address_of());

        if (SQLITE_OK != result)
        {
            throw sql_exception { result, sqlite3_errmsg(local.get()) };
        }

        handle = move(local);
    }

    auto execute(char const * text) -> void
    {
        ASSERT(handle);

        auto const result = sqlite3_exec(handle.get(),
                                         text,
                                         nullptr,
                                         nullptr,
                                         nullptr);

        if (SQLITE_OK != result)
        {
            throw sql_exception { result, sqlite3_errmsg(handle.get()) };
        }
    }
};

struct statement
{
    statement_handle handle;

    auto prepare(connection const & c,
                 char const * text) -> void
    {
        handle.reset();

        auto const result = sqlite3_prepare_v2(c.handle.get(),
                                               text,
                                               strlen(text),
                                               handle.get_address_of(),
                                               nullptr);

        if (SQLITE_OK != result)
        {
            throw sql_exception { result, sqlite3_errmsg(c.handle.get()) };
        }
    }

    auto step() -> bool
    {
        ASSERT(handle);

        auto const result = sqlite3_step(handle.get());

        if (result == SQLITE_ROW) return true;

        if (result == SQLITE_DONE) return false;

        throw sql_exception { result, sqlite3_errmsg(sqlite3_db_handle(handle.get())) };
    }

    auto get_int(int const column = 0) -> int
    {
        return sqlite3_column_int(handle.get(), column);
    }

    auto get_string(int const column = 0) -> char const *
    {
        return reinterpret_cast<char const *>(sqlite3_column_text(handle.get(), column));
    }
};

auto main() -> int
{
    try
    {
        connection c;

        c.open("C:\\Sample\\Chickens.db");

        c.execute("drop table Hens");

        c.execute("create table Hens ( Id int primary key, Name nvarchar(100) not null )");

        c.execute("insert into Hens (Id, Name) values (1, 'Rowena'), (2, 'Henrietta'), (3, 'Constance')");

        statement s;

        s.prepare(c, "select Id from Hens where Name = 'Henrietta'");

        if (s.step())
        {
            TRACE(L"Henrietta's Id: %d\n", s.get_int());
        }

        s.prepare(c, "select Name from Hens where Id = 3");

        if (s.step())
        {
            TRACE(L"Hen 3 is called %S\n", s.get_string());
        }

        s.prepare(c, "select Id, Name from Hens order by Id desc");

        while (s.step())
        {
            TRACE(L"%d %S\n",
                  s.get_int(0),
                  s.get_string(1));
        }
    }
    catch (sql_exception const & e)
    {
        TRACE(L"%d %S\n", e.code, e.message.c_str());
    }
}
