// Created by moisrex on 11/7/19.
#include <gtest/gtest.h>
#include <webpp/valves/valve.h>

using namespace webpp;

TEST(ValvesTests, Creation) {
    constexpr auto v = method("GET") or method("POST");
}
