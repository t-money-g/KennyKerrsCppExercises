#include "Precompiled.h"

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

auto main() -> int
{
}
