// Created by moisrex on 11/04/22.
#include "../core/include/webpp/http/request_body.hpp"
#include "../core/include/webpp/http/routes/context.hpp"
#include "../core/include/webpp/traits/default_traits.hpp"
#include "../core/include/webpp/views/mustache_view.hpp"
#include "../core/include/webpp/views/view_concepts.hpp"
#include "../core/include/webpp/views/view_manager.hpp"
#include "common_pch.hpp"


using namespace webpp;
using namespace webpp::views;

static_assert(View<mustache_view<default_traits>>);
static_assert(View<file_view<default_traits>>);
// static_assert(View<json_view<default_traits>>);
static_assert(ViewManager<view_manager<default_traits>>);


using string_type        = traits::general_string<default_traits>;
using mustache_view_type = mustache_view<default_traits>;
using data_type          = typename mustache_view_type::data_type;
using variable_type      = typename data_type::value_type;

TEST(TheViews, MustacheView) {
    enable_owner_traits<default_traits> et;

    mustache_view_type v{et};
    v.scheme("My name is {{name}}");
    string_type str;
    auto        data = object::make_general<data_type>(et);
    data.push_back(variable_type{et, "name", "moisrex"});
    v.render(str, data);
    EXPECT_EQ(str, "My name is moisrex");
    data.clear();
    str.clear();
    data.push_back(variable_type{et, "name", "The Moisrex"});
    v.render(str, data);
    EXPECT_EQ(str, "My name is The Moisrex");
}


TEST(TheViews, ViewManagerTest) {

    enable_owner_traits<default_traits> et;

    view_manager<default_traits> man{et};
    man.view_roots.push_back("../tests/assets");
    man.view_roots.push_back("../tests");
    man.view_roots.push_back("./tests");
    man.view_roots.push_back("./tests/assets");

    std::string roots;
    for (auto const& root : man.view_roots) {
        roots += std::filesystem::absolute(root).lexically_normal().string() + ", ";
    }

    auto data = object::make_general<data_type>(et);
    data.push_back(variable_type{et, "name", "moisrex"});
    const auto res = man.mustache("assets/hello-world", data);
    EXPECT_EQ(res, "Hello, moisrex") << "Check out the logs, it shouldn't be empty if the file was found.\n" << roots;
}
