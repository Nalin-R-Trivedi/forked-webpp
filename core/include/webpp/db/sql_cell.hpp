#ifndef WEBPP_DATABASE_SQL_CELL_HPP
#define WEBPP_DATABASE_SQL_CELL_HPP

#include "../common/meta.hpp"
#include "../std/string.hpp"
#include "sql_concepts.hpp"

namespace webpp::sql {

    template <SQLStatement SQLStmtType>
    struct sql_cell {
        using statement_type = SQLStmtType;

      private:
        statement_type& stmt;

      public:
        sql_cell(statement_type& stmt_ref) noexcept : stmt(stmt_ref) {}


        template <istl::String StrT = stl::string>
        [[nodiscard]] inline auto as_string() const {
            auto str = object::make_general<StrT>(*this);
            stmt.as_string(str);
            return str;
        }


        template <typename T>
        operator T() const {
            if constexpr (istl::String<T>) {
                return as_string<T>();
            } else if constexpr (stl::integral<T>) {
                return static_cast<T>(stmt.as_number());
            } else {
                static_assert_false(T, "Cannot handle this data type");
            }
        }
    };
} // namespace webpp::sql

#endif // WEBPP_DATABASE_SQL_CELL_HPP
