// Created by moisrex on 12/10/19.

#include "../core/include/webpp/utils/functional.hpp"

#include "../core/include/webpp/std/functional.hpp"
#include "../core/include/webpp/utils/debounce.hpp"
#include "common_pch.hpp"



using namespace webpp;

void test(int limit) {
    static auto i = 0;
    i++;
    EXPECT_TRUE(i < limit - 1) << "i is: " << i << "; limit: " << limit;
}

struct ConstMyCallable {
    void operator()(int limit) const {
        static auto i = 0;
        i++;
        EXPECT_TRUE(i < limit - 1) << "i is: " << i << "; limit: " << limit;
    }
};

struct MyCallable {
    int  i = 0;
    auto operator()(int limit) {
        i++;
        EXPECT_TRUE(i < limit - 1) << "i is: " << i << "; limit: " << limit;
        return i;
    }
};

TEST(FunctionalTests, DebouncedFunctions) {
    using namespace std::chrono;

    constexpr auto limit = 1000;

    auto checking_deduction_for_function_pointers = debounce(test);
    auto debounced_test                           = debounce(milliseconds(10), test);
    for (int i = 0; i < limit; i++)
        debounced_test(limit);

    // lambdas

    auto lambda_test = debounce([](int _limit) {
        static auto i = 0;
        i++;
        EXPECT_LT(i, _limit);
    });

    for (int i = 0; i < limit; i++)
        lambda_test(limit);

    // class

    debounce_t<MyCallable> debounced_class(milliseconds(1));
    for (int i = 0; i < limit; i++) {
        auto res = debounced_class(limit);
        EXPECT_LT(res, limit) << res;
    }

    const debounce<ConstMyCallable> const_debounced_class;
    for (int i = 0; i < limit; i++)
        const_debounced_class(limit);
}


TEST(FunctionalTests, TrailingMode) {
    // using dtype = debounce_type;

    debounce_trailing debounced_lambda([] {

    });
}


TEST(FunctionalTests, FunctionWithAllocators) {
    istl::function<int()> func = [i = 0]() mutable {
        return ++i;
    };
    EXPECT_EQ(1, func());
    EXPECT_EQ(2, func());
    EXPECT_EQ(3, func());
    func = [] {
        return 2;
    };
    EXPECT_EQ(2, func());
    EXPECT_EQ(2, func());
    func = [i = 0]() mutable {
        return ++i;
    };
    EXPECT_EQ(1, func());
    EXPECT_EQ(2, func());
    EXPECT_EQ(3, func());
    func = +[] {
        return -369;
    };
    EXPECT_EQ(-369, func());

    istl::function<int()> func_copy = func;
    EXPECT_EQ(func_copy(), -369);
    istl::function<int()> func_clone;
    func_clone = func_copy;
    EXPECT_EQ(func_clone(), -369);
}
