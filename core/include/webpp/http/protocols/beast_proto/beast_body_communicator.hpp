// Created by moisrex on 12/15/22.

#ifndef WEBPP_BEAST_BODY_COMMUNICATOR_HPP
#define WEBPP_BEAST_BODY_COMMUNICATOR_HPP

#include "../../../std/type_traits.hpp"
#include "../../../traits/traits.hpp"
#include "beast_string_body.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>


namespace webpp::http::beast_proto {

    /**
     * The middle man between beast request body and the framework's request body.
     *
     * This type implements HTTPRequestBodyCommunicator
     */
    template <typename ProtocolType>
    struct beast_request_body_communicator {
        using protocol_type    = ProtocolType;
        using traits_type      = typename protocol_type::traits_type;
        using char_type        = traits::char_type<traits_type>;
        using byte_type        = stl::byte;
        using size_type        = stl::streamsize;
        using http_worker_type = typename protocol_type::http_worker_type;

        // if you change, remember to sync these types with beast's http_worker's types, I'm not using
        // http_worker_type directly because it's a still an incomplete type at this point.
        using string_type         = traits::general_string<traits_type>;
        using allocator_pack_type = traits::allocator_pack_type<traits_type>;
        using char_allocator_type =
          typename allocator_pack_type::template best_allocator<alloc::sync_pool_features, char>;
        using beast_body_type = string_body_of<string_type>;
        using beast_request_parser_type =
          boost::beast::http::request_parser<beast_body_type, char_allocator_type>;
        using request_type = typename beast_request_parser_type::value_type;
        using request_ptr  = stl::add_pointer_t<request_type>;


        void set_beast_parser(beast_request_parser_type& input_parser) noexcept {
            request = &input_parser.get();
        }

        [[nodiscard]] size_type read(byte_type* data, size_type count) const {
            return stl::copy_n(request->body().data(), count, data) - request->body().data();
        }

        [[nodiscard]] size_type read(byte_type* data) const {
            return stl::copy_n(request->body().data(), size(), data) - request->body().data();
        }

        [[nodiscard]] size_type size() const noexcept {
            return request->payload_size();
        }

      private:
        request_ptr request = nullptr;
    };


} // namespace webpp::http::beast_proto

#endif // WEBPP_BEAST_BODY_COMMUNICATOR_HPP
