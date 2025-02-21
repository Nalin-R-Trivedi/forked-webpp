// Created by moisrex on 9/30/22.

#ifndef WEBPP_COUNTER_MODEL_HPP
#define WEBPP_COUNTER_MODEL_HPP

#include <webpp/db/sql_database.hpp>
#include <webpp/db/sqlite/sqlite.hpp>

namespace website {
    using namespace webpp;
    using namespace webpp::sql;

    struct counter_model {
        sql_database<sqlite> db;
        stl::string          ip;

        counter_model() {
            auto counter = db.create_query().table("counter");
            counter["id"].primary().number().not_null();
            counter["ip"].string().unique().not_null();
            counter["val"].number().not_null().default_value(0);
            counter.create_if_not_exists();
        };

        bool increment() {
            auto counter = db.tables["counter"].where("ip", ip);
            counter++;
            return counter.update_or_set();
        }

        stl::size_t current() {
            return db.tables["counter"].where("ip", ip).select("value").first();
        }
    };

} // namespace website
#endif // WEBPP_COUNTER_MODEL_HPP
