#include "app.hpp"

#include <webpp/http/protocols/beast.hpp>

int main() {
    webpp::http::beast<website::app> app;
    app
      .address("127.0.0.1") // listen on localhost
      .port(8080);          // on http port
    return app();
}
