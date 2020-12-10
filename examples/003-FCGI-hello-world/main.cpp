#include "../../core/include/webpp/http/interfaces/fcgi.hpp"
#include "./app.h"

int main() {
    using namespace webpp;
    http<fcgi<std_traits, app>> my_app;
    my_app();
    return 0;
}
