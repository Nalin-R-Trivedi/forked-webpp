#ifndef WEBPP_SQL_SQLITE_SQLITE_HPP
#define WEBPP_SQL_SQLITE_SQLITE_HPP

#include "../../traits/default_traits.hpp"
#include "../../traits/traits.hpp"
#include "sqlite_connection.hpp"
#include "sqlite_grammar.hpp"

namespace webpp::sql {


    template <Traits TraitsType>
    struct basic_sqlite : public sqlite_connection {
        using traits_type  = TraitsType;
        using grammar_type = sqlite_grammar;

        using sqlite_connection::sqlite_connection;
    };

    using sqlite = basic_sqlite<default_traits>;
    // todo: add pmr::sqlite

} // namespace webpp::sql

#endif
