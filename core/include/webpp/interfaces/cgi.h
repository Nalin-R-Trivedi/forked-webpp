#ifndef WEBPP_CGI_H
#define WEBPP_CGI_H

#include "../router.h"
#include "../std/string_view.h"
#include "../utils/casts.h"
#include "../utils/strings.h"
#include "../utils/traits.h"
#include "basic_interface.h"

#include <cctype>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <sstream>

// TODO: use GetEnvironmentVariableA for Windows operating system
extern char** environ;

namespace webpp {


    template <typename Traits = std_traits>
    struct cgi : public basic_interface<Traits, cgi<Traits>> {
      public:
        using traits     = Traits;
        using str_view_t = typename Traits::string_view_type;
        using str_t      = typename Traits::string_type;
        using ostream_t  = typename Traits::ostream_type;

        cgi() noexcept {
            // I'm not using C here; so why should I pay for it!
            // And also the user should not use cin and cout. so ...
            std::ios::sync_with_stdio(false);
        }

        /**
         * Read the body of the string
         * @param data
         * @param length
         * @return
         */
        static std::streamsize read(char*           data,
                                    std::streamsize length) noexcept {
            std::cin.read(data, length);
            return std::cin.gcount();
        }

        /**
         * Send the stream to the user
         * @param stream
         */
        static void write(ostream_t& stream) noexcept {
            // TODO: check if you need to ignore the input or not

            // I think o-stream is not readable so we cannot do this:
            // https://stackoverflow.com/questions/15629886/how-to-write-ostringstream-directly-to-cout
            std::cout
              << stream.rdbuf(); // TODO: test this, I don't trust myself :)
        }

        /**
         * Send data to the user
         * @param data
         * @param length
         */
        static void write(char const* data, std::streamsize length) noexcept {
            std::cout.write(data, length);
        }

        /**
         * Get the environment value safely
         * @param key
         * @return
         */
        [[nodiscard]] static std::string_view env(char const* key) noexcept {
            if (auto value = getenv(key))
                return value;
            return {};
        }

        /**
         * Get a specific header by it's name
         * @param name
         * @return
         */
        [[nodiscard]] static std::string_view
        header(std::string name) noexcept {
            // fixme: check if this is all we have to do or we have to do more too:
            std::transform(name.begin(), name.end(), name.begin(),
                           [](auto const& c) {
                               if (c == '-')
                                   return '_';
                               return static_cast<char>(std::toupper(c));
                           });

            name.insert(0, "HTTP_");
            return env(name.c_str());
        }

        /**
         * Get a list of headers as a string
         */
        [[nodiscard]] static std::string_view headers() noexcept {
            // we can do this only in CGI, we have to come up with new ways for
            // long-running protocols:
            static std::string headers_cache;
            if (headers_cache.empty()) {
                // TODO: this code won't work on windows. Change when you are worried
                // about windows
                for (auto it = ::environ; *it; it++) {
                    std::string_view h{*it};
                    if (starts_with(h, "HTTP_")) {
                        headers_cache.append(h.substr(5));
                        // FIXME: decide if you need to convert _ to - or not.
                    }
                }
            }
            return headers_cache;
        }

        /**
         * Get the full body as a string_view
         */
        [[nodiscard]] static str_view_t body() noexcept {
            // again, we can do this only in cgi protocol not in other
            // interfaces:
            static str_t body_cache;
            if (body_cache.empty()) {
                if (auto content_length_str = env("CONTENT_LENGTH");
                    !content_length_str.empty()) {
                    // now we know how much content the user is going to send
                    // so we just create a buffer with that size
                    auto content_length = to_uint(content_length_str);

                    char* buffer = new char[content_length];
                    std::cin.rdbuf()->pubsetbuf(buffer, sizeof(buffer));
                } else {
                    // we don't know how much the user is going to send. so we
                    // use a small size buffer:

                    // TODO: add something here
                }
            }
            return body_cache;
        }


        void operator()() noexcept {
            webpp::request_t<traits, cgi<traits>> req;
            auto                                  res = router(req);
            res.calculate_default_headers();
            auto header_str = res.header.str();
            auto str        = res.body.str();

            // From RFC: https://tools.ietf.org/html/rfc3875
            // Send status code:
            // Status         = "Status:" status-code SP reason-phrase NL
            // status-code    = "200" | "302" | "400" | "501" | extension-code
            // extension-code = 3digit
            // reason-phrase  = *TEXT

            std::stringstream status_line;
            status_line << "Status: " << res.header.status_code() << " "
                        << status_reason_phrase(res.header.status_code())
                        << "\r\n";

            auto _status_line_str = status_line.str();
            write(_status_line_str.data(), _status_line_str.size());

            write(header_str.data(), header_str.size());
            write("\r\n", 2);
            write(str.data(), str.size());
        }
    };

    /**
     * Specializing the request_t<cgi> methods so the user can use them.
     * The request_t class is one of the important classes which that means It's
     * going to be used a lot so, we will do everything (even if it means we
     * have to copy and paste lots of codes to make it happen) to make sure
     * the user is able to use this class properly and easily.
     */
    template <typename Traits>
    struct request_t<Traits, cgi<Traits>> : public basic_request_t {
        using super = cgi<Traits>;

        /**
         * @brief get the server's software
         * @details Name and version of the information server software
         * answering the request (and running the gateway). Format:
         * name/version.
         * @example SERVER_SOFTWARE=Apache/2.4.41 (Unix) OpenSSL/1.1.1d
         */
        [[nodiscard]] std::string_view server_software() const noexcept {
            return super::env("SERVER_SOFTWARE");
        }

        /**
         * @brief get the server name
         * @details Server's hostname, DNS alias, or IP address as it appears in
         * self-referencing URLs.
         * @example SERVER_NAME=localhost
         */
        [[nodiscard]] std::string_view server_name() const noexcept {
            return super::env("SERVER_NAME");
        }

        /**
         * @brief get the gateway interface environment variable
         * @details CGI specification revision with which this server complies.
         * Format: CGI/revision.
         * @example GATEWAY_INTERFACE=CGI/1.1
         */
        [[nodiscard]] std::string_view gateway_interface() const noexcept {
            return super::env("GATEWAY_INTERFACE");
        }

        /**
         * @brief get the server protocol
         * @details Name and revision of the information protocol this request
         * came in with. Format: protocol/revision.
         * @example SERVER_PROTOCOL=HTTP/1.1
         */
        [[nodiscard]] std::string_view server_protocol() const noexcept {
            return super::env("SERVER_PROTOCOL");
        }

        /**
         * @brief get the port that the server is listening on
         * @details Port number to which the request was sent.
         */
        [[nodiscard]] std::string_view server_port() const noexcept {
            return super::env("SERVER_PORT");
        }

        /**
         * @brief Get the method
         * @details Method with which the request was made. For HTTP, this is
         * Get, Head, Post, and so on.
         */
        [[nodiscard]] std::string_view request_method() const noexcept {
            return super::env("REQUEST_METHOD");
        }

        /**
         * @brief get the path info
         * @details Extra path information, as given by the client. Scripts can
         * be accessed by their virtual pathname, followed by extra information
         * at the end of this path. The extra information is sent as PATH_INFO.
         * @example PATH_INFO=/hello/world
         */
        [[nodiscard]] std::string_view path_info() const noexcept {
            return super::env("PATH_INFO");
        }

        /**
         * @brief get the path translated
         * @details Translated version of PATH_INFO after any
         * virtual-to-physical mapping.
         * @example PATH_TRANSLATED=/srv/http/hello/world
         */
        [[nodiscard]] std::string_view path_translated() const noexcept {
            return super::env("PATH_TRANSLATED");
        }

        /**
         * @brief get the script name
         * @details Virtual path to the script that is executing; used for
         * self-referencing URLs.
         * @example SCRIPT_NAME=/cgi-bin/one.cgi
         */
        [[nodiscard]] std::string_view script_name() const noexcept {
            return super::env("SCRIPT_NAME");
        }

        /**
         * @brief get the query string
         * @details Query information that follows the ? in the URL that
         * referenced this script.
         */
        [[nodiscard]] std::string_view query_string() const noexcept {
            return super::env("QUERY_STRING");
        }

        /**
         * @brief get the remote host
         * @details Hostname making the request. If the server does not have
         * this information, it sets REMOTE_ADDR and does not set REMOTE_HOST.
         */
        [[nodiscard]] std::string_view remote_host() const noexcept {
            return super::env("REMOTE_HOST");
        }

        /**
         * @brief get the ip address of the user
         * @details IP address of the remote host making the request.
         */
        [[nodiscard]] std::string_view remote_addr() const noexcept {
            return super::env("REMOTE_ADDR");
        }

        /**
         * @brief get the auth type
         * @details If the server supports user authentication, and the script
         * is protected, the protocol-specific authentication method used to
         * validate the user.
         */
        [[nodiscard]] std::string_view auth_type() const noexcept {
            return super::env("AUTH_TYPE");
        }

        /**
         * @brief get the remote user or auth user value (both should be the
         * same)
         * @details If the server supports user authentication, and the script
         * is protected, the username the user has authenticated as. (Also
         * available as AUTH_USER.)
         */
        [[nodiscard]] std::string_view remote_user() const noexcept {
            if (auto a = super::env("REMOTE_USER"); !a.empty())
                return a;
            return super::env("AUTH_USER");
        }

        /**
         * @brief get the remote user or auth user value (both should be the
         * same)
         * @details If the server supports user authentication, and the script
         * is protected, the username the user has authenticated as. (Also
         * available as AUTH_USER.)
         */
        [[nodiscard]] std::string_view auth_user() const noexcept {
            if (auto a = super::env("AUTH_USER"); !a.empty())
                return a;
            return super::env("REMOTE_USER");
        }

        /**
         * @brief get the remote ident
         * @details If the HTTP server supports RFC 931 identification, this
         * variable is set to the remote username retrieved from the server. Use
         * this variable for logging only.
         */
        [[nodiscard]] std::string_view remote_ident() const noexcept {
            return super::env("REMOTE_IDENT");
        }

        /**
         * @brief returns the request scheme (http/https/...)
         */
        [[nodiscard]] std::string_view request_scheme() const noexcept {
            return super::env("REQUEST_SCHEME");
        }

        /**
         * @brief get the user's port number
         */
        [[nodiscard]] std::string_view remote_port() const noexcept {
            return super::env("REMOTE_PORT");
        }

        /**
         * @brief get the ip address that the server is listening on
         */
        [[nodiscard]] std::string_view server_addr() const noexcept {
            return super::env("SERVER_ADDR");
        }

        /**
         * @brief get the request uri
         */
        [[nodiscard]] std::string_view request_uri() const noexcept {
            return super::env("REQUEST_URI");
        }

        /**
         * @brief get the content_type
         * @details For queries that have attached information, such as HTTP
         * POST and PUT, this is the content type of the data.
         */
        [[nodiscard]] std::string_view content_type() const noexcept {
            return super::env("CONTENT_LENGTH");
        }

        /**
         * @brief get the content length
         * @details Length of the content as given by the client.
         */
        [[nodiscard]] std::string_view content_length() const noexcept {
            return super::env("CONTENT_LENGTH");
        }

        /**
         * @brief get the document root environment value
         * @details The root directory of your server
         */
        [[nodiscard]] std::string_view document_root() const noexcept {
            return super::env("DOCUMENT_ROOT");
        }

        /**
         * @brief get the https environment value
         * @return "on" if the user used HTTPS protocol
         */
        [[nodiscard]] std::string_view https() const noexcept {
            return super::env("HTTPS");
        }

        /**
         * @brief get the server admin environment value
         * @return probabely the administrator's email address
         */
        [[nodiscard]] std::string_view server_admin() const noexcept {
            return super::env("SERVER_ADMIN");
        }

        /**
         * @brief get the path environment variable
         * @details The system path your server is running under
         */
        [[nodiscard]] std::string_view path() const noexcept {
            return super::env("PATH");
        }

        /**
         * @brief get the script_filename of the environment variables
         * @details The full pathname of the current CGI
         */
        [[nodiscard]] std::string_view script_filename() const noexcept {
            return super::env("SCRIPT_FILENAME");
        }

        /**
         * @brief get a single header
         * @param name
         */
        [[nodiscard]] std::string_view
        header(std::string_view const& name) const noexcept {
            return super::header(std::string(name));
        }

        /**
         * @brief get all of the headers as a string_view
         * @details this method in CGI interfaces will have to do more than
         * in the our own version because data has already been parsed by the
         * CGI server; and we have to recreate it based on the environment
         * variables.
         */
        [[nodiscard]] std::string_view headers() const noexcept {
            return super::headers();
        }

        /**
         * @brief get the whole body as a string_view
         * @details this method will return raw string values of the body of
         * the request and will not parse it. Parsing it is another methods'
         * problem that might even use this function as the source.
         */
        [[nodiscard]] std::string_view body() const noexcept {
            return super::body();
        }
    };

} // namespace webpp

#endif // WEBPP_CGI_H
