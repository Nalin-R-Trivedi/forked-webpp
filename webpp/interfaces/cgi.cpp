#include "cgi.h"
#include "../http/request.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <iostream>

using namespace webpp;

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



int cgi::remote_port() const noexcept {
    return atoi(env("REMOTE_PORT")); // default value: 0
}

int cgi::server_port() const noexcept {
    return atoi(env("SERVER_PORT")); // default value: 0
}
char const* cgi::header(std::string str) const noexcept {
    std::transform(str.begin(), str.end(), str.begin(),
                   static_cast<int (*)(int)>(&std::toupper));
    str.insert(0, "HTTP_");
    return env(str.c_str());
}

// void cgi::run(router<cgi>& _router) noexcept {
//    auto self = std::make_shared<cgi>(this);
    //    webpp::request<webpp::cgi> req(self);
    //    auto res = _router.run(req);
    //    std::ios_base::sync_with_stdio(false); // TODO: write tests for this
    //    part for (auto const& [attr, value] : res.headers()) {
    //        std::cout << attr << ": " << value << "\r\n";
    //    }
    //    std::cout << "\r\n" << res.body();
//}

