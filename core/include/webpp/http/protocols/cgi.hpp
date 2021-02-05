#ifndef WEBPP_CGI_H
#define WEBPP_CGI_H

#include "../../convert/casts.hpp"
#include "../../memory/object.hpp"
#include "../../std/string_view.hpp"
#include "../../strings/to_case.hpp"
#include "../../traits/default_traits.hpp"
#include "../app_wrapper.hpp"
#include "../response.hpp"
#include "./cgi_request.hpp"
#include "common/common_protocol.hpp"
#include "webpp/application/request.hpp"

#include <cctype>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>

namespace webpp::http {

    template <Application App, Traits TraitsType = default_traits, ExtensionList EList = empty_extension_pack>
    struct cgi : public common_protocol<TraitsType, App, EList> {
        using traits_type          = TraitsType;
        using application_type     = App;
        using extension_list       = stl::remove_cvref_t<EList>;
        using str_view_type        = traits::string_view<traits_type>;
        using char_type            = traits::char_type<traits_type>;
        using str_type             = traits::general_string<traits_type>;
        using local_allocator_type = traits::local_allocator<traits_type, char_type>;
        using common_protocol_type = common_protocol<TraitsType, App, EList>;
        using app_wrapper_type     = typename common_protocol_type::app_wrapper_type;
        using request_type = simple_request<traits_type, extension_list, cgi_request, local_allocator_type>;

        static_assert(HTTPRequest<request_type>,
                      "Web++ Internal Bug: request_type is not a match for Request concept.");

        static_assert(ApplicationAcceptingRequest<app_wrapper_type, request_type>,
                      "Your application type can't be called with a request type of our choosing or "
                      "its response is not of a valid response type.");

      private:
        using super = common_protocol<TraitsType, App, EList>;

        void ctor() noexcept {
            // I'm not using C here; so why should I pay for it!
            // And also the user should not use cin and cout. so ...
            stl::ios::sync_with_stdio(false);
        }

      public:
        template <typename... Args>
        cgi(Args&&... args) : super{stl::forward<Args>(args)...} {
            ctor();
        }

        /**
         * Read the body of the string
         */
        static stl::streamsize read(char* data, stl::streamsize length) noexcept {
            stl::cin.read(data, length);
            return stl::cin.gcount();
        }

        /**
         * Send the stream to the user
         * @param stream
         */
        static void write(auto& stream) noexcept {
            // TODO: check if you need to ignore the input or not

            // I think o-stream is not readable so we cannot do this:
            // https://stackoverflow.com/questions/15629886/how-to-write-ostringstream-directly-to-cout
            stl::cout << stream.rdbuf(); // TODO: test this, I don't trust myself :)
        }

        /**
         * Send data to the user
         */
        static void write(char const* data, stl::streamsize length) noexcept {
            stl::cout.write(data, length);
        }

        /**
         * Get the environment value safely
         */
        [[nodiscard]] static stl::string_view env(char const* key) noexcept {
            return request_type::env(key);
        }


        int operator()() noexcept {
            // we're putting the request on local allocator; yay us :)
            request_type      req{this->alloc_pack.template local_allocator<char_type>(), *this};
            HTTPResponse auto res = this->app(req);
            res.calculate_default_headers();
            const auto header_str = res.headers.str();
            const auto str        = res.body.str();

            // From RFC: https://tools.ietf.org/html/rfc3875
            // Send status code:
            // Status         = "Status:" status-code SP reason-phrase NL
            // status-code    = "200" | "302" | "400" | "501" | extension-code
            // extension-code = 3digit
            // reason-phrase  = *TEXT

            // todo: give the user the ability to change the status phrase
            auto status_line = object::make_local<str_type>(this->alloc_pack);
            stl::format_to(stl::back_inserter(status_line), "Status: {} {}\r\n",
                           res.headers.status_code,
                           http::status_code_reason_phrase(res.headers.status_code)
                           );
            write(status_line.data(), status_line.size());

            write(header_str.data(), header_str.size());
            write("\r\n", 2);
            write(str.data(), str.size());
            return EXIT_SUCCESS;
        }
    };

    template <typename App>
    cgi(App&&) -> cgi<App, default_traits, empty_extension_pack>;

    // fixme: implement these too:
    //    AUTH_PASSWORD
    //    AUTH_TYPE
    //    AUTH_USER
    //    CERT_COOKIE
    //    CERT_FLAGS
    //    CERT_ISSUER
    //    CERT_KEYSIZE
    //    CERT_SECRETKEYSIZE
    //    CERT_SERIALNUMBER
    //    CERT_SERVER_ISSUER
    //    CERT_SERVER_SUBJECT
    //    CERT_SUBJECT
    //    CF_TEMPLATE_PATH
    //    CONTENT_LENGTH
    //    CONTENT_TYPE
    //    CONTEXT_PATH
    //    GATEWAY_INTERFACE
    //    HTTPS
    //    HTTPS_KEYSIZE
    //    HTTPS_SECRETKEYSIZE
    //    HTTPS_SERVER_ISSUER
    //    HTTPS_SERVER_SUBJECT
    //    HTTP_ACCEPT
    //    HTTP_ACCEPT_ENCODING
    //    HTTP_ACCEPT_LANGUAGE
    //    HTTP_CONNECTION
    //    HTTP_COOKIE
    //    HTTP_HOST
    //    HTTP_REFERER
    //    HTTP_USER_AGENT
    //    QUERY_STRING
    //    REMOTE_ADDR
    //    REMOTE_HOST
    //    REMOTE_USER
    //    REQUEST_METHOD
    //    SCRIPT_NAME
    //    SERVER_NAME
    //    SERVER_PORT
    //    SERVER_PORT_SECURE
    //    SERVER_PROTOCOL
    //    SERVER_SOFTWARE
    //    WEB_SERVER_API


} // namespace webpp::http

#endif // WEBPP_CGI_H
