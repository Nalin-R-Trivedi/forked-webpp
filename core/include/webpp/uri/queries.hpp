// Created by moisrex on 11/4/20.

#ifndef WEBPP_QUERIES_HPP
#define WEBPP_QUERIES_HPP

#include "../std/map.hpp"

namespace webpp::uri {

    // todo: complete this and use it in the uri_queries
    template <typename StringType = stl::string, typename AllocType = typename StringType::allocator_type>
    struct basic_queries : stl::map<StringType, StringType, stl::less<StringType>, AllocType> {
        using super = stl::map<StringType, StringType, stl::less<StringType>, AllocType>;

        template <typename ...Args>
        basic_queries(Args&& ...args) : super {stl::forward<Args>(args)...} {}


        /**
         * Get the raw string non-decoded size
         */
        [[nodiscard]] stl::size_t raw_string_size() const noexcept {
            return stl::reduce(this->cbegin(), this->cend(), 0ull, [] (string_type const& item) {
              return item.first.size() + item.second.size();
            }) + (this->size() * 2) - 2;
        }


    };

}

#endif // WEBPP_QUERIES_HPP
