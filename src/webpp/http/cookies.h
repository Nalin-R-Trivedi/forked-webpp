#ifndef WEBPP_COOKIES_H
#define WEBPP_COOKIES_H

/*
 * Documentations about cookies:
 *
 *
 *  Standard RFC:
 *    https://tools.ietf.org/html/rfc6265
 *  Set-Cookie:
 *    https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Set-Cookie
 *  Cookie:
 *    https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Cookie
 *  Date Format:
 *    https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Date
 *
 *
 * -----------------------------------------------------------------------------
 * Other close implementations:
 *
 *  Drogon:
 *    https://github.com/an-tao/drogon/blob/master/lib/inc/drogon/Cookie.h
 *    https://github.com/an-tao/drogon/blob/master/lib/src/Cookie.cc
 *
 *  Pistache:
 *    https://github.com/oktal/pistache/blob/master/include/pistache/cookie.h
 *    https://github.com/oktal/pistache/blob/master/src/common/cookie.cc
 *
 *  Cgicc:
 *    https://github.com/cmakified/cgicc/blob/master/cgicc/HTTPCookie.h
 *    https://github.com/cmakified/cgicc/blob/master/cgicc/HTTPCookie.cpp
 */

#include <chrono>
#include <map>
#include <string>
#include <unordered_set>

namespace webpp {

    struct cookie_hash;

    class cookie {
      public:
        enum class same_site_value { NONE, LAX, STRICT };
        std::map<std::string, std::string> attrs;

      private:
        std::string _name = "";
        std::string _value = "";
        std::string _comment = "";
        std::string _domain = "";
        std::string _path = "";
        std::chrono::time_point<std::chrono::system_clock> _expires =
            std::chrono::system_clock::now() +
            std::chrono::duration<int, std::ratio<60 * 60 * 24 * 365>>();
        unsigned long _max_age;
        bool _secure = false;
        bool _host_only = false;
        bool _encrypted = false;
        bool _prefix = false;
        same_site_value _same_site = same_site_value::NONE;

      public:
        /**
         * empty cookie
         */
        cookie() = default;
        cookie(std::string source) noexcept;
        cookie(std::string __name, std::string __value) noexcept;

        inline decltype(_name) const& name() const noexcept { return _name; }
        inline decltype(_value) const& value() const noexcept { return _value; }
        inline decltype(_comment) const& commnet() const noexcept {
            return _comment;
        }
        inline decltype(_domain) const& domain() const noexcept {
            return _domain;
        }
        inline decltype(_max_age) const& max_age() const noexcept {
            return _max_age;
        }
        inline decltype(_secure) const& secure() const noexcept {
            return _secure;
        }
        inline decltype(_host_only) const& host_only() const noexcept {
            return _host_only;
        }
        inline decltype(_prefix) const& prefix() const noexcept {
            return _prefix;
        }
        inline decltype(_expires) const& expires() const noexcept {
            return _expires;
        }
        inline decltype(_same_site) const& same_site() const noexcept {
            return _same_site;
        }
        inline decltype(_path) const& path() const noexcept { return _path; }

        cookie& name(std::string __name) noexcept;
        cookie& value(std::string __value) noexcept;

        inline cookie& comment(std::string const& __comment) noexcept {
            _comment = __comment;
            return *this;
        }

        inline cookie& comment(std::string&& __comment) noexcept {
            _comment = std::move(__comment);
            return *this;
        }

        inline cookie& domain(std::string const& __domain) noexcept {
            _domain = __domain;
            return *this;
        }
        inline cookie& domain(std::string&& __domain) noexcept {
            _domain = std::move(__domain);
            return *this;
        }

        inline cookie& path(std::string const& __path) noexcept {
            _path = __path;
            return *this;
        }

        inline cookie& path(std::string&& __path) noexcept {
            _path = std::move(__path);
            return *this;
        }

        inline cookie& max_age(decltype(_max_age)&& __max_age) noexcept {
            _max_age = std::move(__max_age);
            return *this;
        }

        inline cookie& max_age(decltype(_max_age) const& __max_age) noexcept {
            _max_age = __max_age;
            return *this;
        }
        inline cookie& prefix(decltype(_prefix) const& __prefix) noexcept {
            _prefix = __prefix;
            return *this;
        }

        inline cookie& prefix(decltype(_prefix)&& __prefix) noexcept {
            _prefix = std::move(__prefix);
            return *this;
        }

        inline cookie& same_site(decltype(_same_site)&& __same_site) noexcept {
            _same_site = std::move(__same_site);
            return *this;
        }

        inline cookie& smae_site(decltype(_same_site)
                                     const& __same_site) noexcept {
            _same_site = __same_site;
            return *this;
        }

        inline bool remove() const noexcept {
            using namespace std::chrono;
            return _expires < system_clock::now();
        }

        inline cookie& remove(bool __remove) noexcept {
            using namespace std::chrono;
            if (__remove) {
                // set the expire date one year before now:
                expires(system_clock::now() -
                        duration<int, std::ratio<60 * 60 * 24 * 365>>(1));
            } else if (remove()) {
                // set the expire date one year from now:
                expires(system_clock::now() +
                        duration<int, std::ratio<60 * 60 * 24 * 365>>(1));
            }
            // remove max-age if it exists because we're going with expires
            max_age(0);
            return *this;
        }

        inline cookie& expires(decltype(_expires)&& __expires) noexcept {
            _expires = std::move(__expires);
            return *this;
        }

        inline cookie& expires(decltype(_expires) const& __expires) noexcept {
            _expires = __expires;
            return *this;
        }

        /**
         * @brief sets exipiration time relative to now.
         */
        template <typename D>
        inline cookie&
        expires_in(std::chrono::duration<D> const& __dur) noexcept {
            _expires = std::chrono::system_clock::now() + __dur;
            return *this;
        }

        inline decltype(_encrypted) encrypted() const noexcept {
            return _encrypted;
        }

        inline cookie& encrypted(decltype(_encrypted) __encrypted) noexcept {
            _encrypted = __encrypted;
            return *this;
        }

        decltype(_value) encrypted_value() const noexcept;

        std::ostream& operator<<(std::ostream& out) const noexcept;
        bool operator==(cookie const& c) const noexcept;
        bool operator<(cookie const& c) const noexcept;
        bool operator>(cookie const& c) const noexcept;
        bool operator<=(cookie const& c) const noexcept;
        bool operator>=(cookie const& c) const noexcept;
        std::string render() const noexcept;

        friend inline void swap(cookie& first, cookie& second) noexcept {
            using std::swap;
            swap(first._name, second._name);
            swap(first._value, second._value);
            swap(first._comment, second._comment);
            swap(first._domain, second._domain);
            swap(first._path, second._path);
            swap(first._max_age, second._max_age);
            swap(first._secure, second._secure);
            swap(first._host_only, second._host_only);
            swap(first._expires, second._expires);
            swap(first._encrypted, second._encrypted);
            swap(first._prefix, second._prefix);
            swap(first._same_site, second._same_site);
        }

        friend struct cookie_hash;
    };

    // hash function of std::unordered_set<webpp::cookie>
    struct cookie_hash {
        using argument_type = webpp::cookie;
        using result_type = std::size_t;
        result_type operator()(argument_type const& c) const noexcept;
    };

    struct cookie_equals {
        bool operator()(const cookie& lhs, const cookie& rhs) const noexcept;
    };

    using cookies =
        std::unordered_set<webpp::cookie, cookie_hash, cookie_equals>;

} // namespace webpp

#endif // WEBPP_COOKIES_H
