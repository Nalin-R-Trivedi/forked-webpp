// Created by moisrex on 12/23/22.

#ifndef WEBPP_HTTP_HEADERS_COMMON_HPP
#define WEBPP_HTTP_HEADERS_COMMON_HPP

#include "../std/tuple.hpp"

#include <algorithm>

namespace webpp::http {

    template <typename Container>
    struct headers_container : public Container {
        using container_type = Container;

        using field_type = typename container_type::field_type;
        using name_type  = typename field_type::name_type;
        using value_type = typename field_type::value_type;

        using Container::Container;

        /**
         * Get an iterator pointing to the field value that holds the specified header name
         */
        [[nodiscard]] constexpr auto iter(name_type name) const noexcept {
            return stl::find_if(this->begin(), this->end(), [name](field_type const& field) noexcept {
                return field.is_name(name);
            });
        }


        /**
         * Get the field value that holds the specified header name
         */
        [[nodiscard]] constexpr stl::optional<field_type> field(name_type name) const noexcept {
            const auto res = iter(name);
            return res == this->end() ? stl::nullopt : *res;
        }


        /**
         * Get the value of a header
         * Returns an empty string if there are no header with that name
         */
        [[nodiscard]] constexpr value_type get(name_type name) const noexcept {
            const auto res = iter(name);
            return res == this->end() ? value_type{} : res->value;
        }

        [[nodiscard]] constexpr value_type operator[](name_type name) const noexcept {
            return get(name);
        }

        /**
         * Check if the specified names are in headers
         * returns stl::tuple<bool, bool, ...> if you give multiple names
         * or
         * returns bool if you give one name
         */
        template <typename... NameType>
        [[nodiscard]] constexpr auto has(NameType&&... name) const noexcept {
            if constexpr (sizeof...(NameType) == 1) {
                return stl::find(this->begin(), this->end(), name...) != this->end();
            } else if constexpr (sizeof...(NameType) > 1) {
                return stl::make_tuple((stl::find(this->begin(), this->end(), name) != this->end())...);
            } else {
                return true;
            }
        }
    };

} // namespace webpp::http

#endif // WEBPP_HTTP_HEADERS_COMMON_HPP
