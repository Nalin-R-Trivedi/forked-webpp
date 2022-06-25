// Created by moisrex on 12/17/20.

#include "../core/include/webpp/std/type_traits.hpp"

#include "../core/include/webpp/std/optional.hpp"
#include "../core/include/webpp/std/tuple.hpp"
#include "common_pch.hpp"

#include <list>
#include <map>
#include <string>

#if __cpp_lib_memory_resource
#    include <memory_resource>
#endif

using namespace std;
using namespace webpp::istl;

///////////////////////////////////////////// replace_parameter /////////////////////////////////////////////

///////////////////////////////////////////
// changing std::string's allocator ///////
///////////////////////////////////////////
#if __cpp_lib_memory_resource
using new_str_t = replace_parameter<string, std::allocator<char>, pmr::polymorphic_allocator<char>>;

static_assert(std::is_same_v<new_str_t, pmr::string>);


using should_be_void = replace_parameter<string, std::allocator<int>, pmr::polymorphic_allocator<char>>;

// static_assert(is_void_v<should_be_void>);

#endif


///////////////////// Tuple ///////////////////


template <typename... T>
struct fake_tuple {};

// tuple_filter
static_assert(is_same_v<filter_parameters_t<is_void, fake_tuple<int, void, int, void, double, void>>,
                        fake_tuple<void, void, void>>);



#if __cpp_lib_memory_resource
using t1  = tuple<int, string, vector<int>, int>;
using t2  = replace_parameter<t1, int, double>;
using t3  = recursively_replace_parameter<t1, int, double>;
using t4  = recursively_replace_templated_parameter<t1, allocator, pmr::polymorphic_allocator>;
using t5  = tuple<vector<string>>;
using t6  = recursively_replace_templated_parameter<t5, allocator, pmr::polymorphic_allocator>;
using t7  = replace_templated_parameter<t1, vector, list>;
using t8  = tuple<map<vector<list<string>>, vector<string>>>;
using t9  = recursively_replace_templated_parameter<t8, allocator, pmr::polymorphic_allocator>;
using t10 = tuple<map<string, string>>;
using t11 = recursively_replace_templated_parameter<t10, allocator, pmr::polymorphic_allocator>;
using t12 = tuple<const allocator<char>>;
using t13 = recursively_replace_templated_parameter<t12, allocator, pmr::polymorphic_allocator>;
using t14 = tuple<const tuple<allocator<char>>>;
using t15 = recursively_replace_templated_parameter<t14, allocator, pmr::polymorphic_allocator>;

static_assert(is_same_v<t2, tuple<double, string, vector<int>, double>>);
static_assert(is_same_v<t3, tuple<double, string, vector<double>, double>>);
static_assert(is_same_v<t4, tuple<int, pmr::string, pmr::vector<int>, int>>);
static_assert(is_same_v<t6, tuple<pmr::vector<pmr::string>>>);
static_assert(is_same_v<t7, tuple<int, string, list<int>, int>>);
static_assert(is_same_v<t13, tuple<const pmr::polymorphic_allocator<char>>>);
static_assert(is_same_v<t15, tuple<const tuple<pmr::polymorphic_allocator<char>>>>);
static_assert(is_same_v<t11, tuple<pmr::map<pmr::string, pmr::string>>>);
static_assert(is_same_v<t9, tuple<pmr::map<pmr::vector<pmr::list<pmr::string>>, pmr::vector<pmr::string>>>>);


template <typename T>
struct int_replacer {
    static constexpr bool value = is_same_v<T, int>;
    using type                  = double;
};

using t16 = recursive_parameter_replacer<t1, int_replacer>;
static_assert(is_same_v<t16, tuple<double, string, vector<double>, double>>);




using ut = unique_parameters<tuple<void, void, int, int, int>>;
static_assert(is_same_v<ut, tuple<void, int>>);


#endif

/// Optional ///

static_assert(Optional<optional<int>>, "Optional doesn't work properly");



/// Last Type ///

template <typename T>
using is_not_integral = negation<is_integral<T>>;

using only_int   = typename last_type<int, double>::template remove<tuple>;
using only_ints  = typename last_type<int, short, string>::template remove_if<tuple, is_not_integral>;
using only_arith = typename last_type<int, double, short, string>::template remove<tuple>;
using only_arith_long =
  typename last_type<int, unsigned, unsigned short, long, long long, unsigned long, double, short, string>::
    template remove<tuple>;

static_assert(is_same_v<only_int, tuple<int>>, "remove bug");
static_assert(is_same_v<only_arith, tuple<int, double, short>>, "remove bug");
static_assert(is_same_v<only_arith_long,
                        tuple<int, unsigned, unsigned short, long, long long, unsigned long, double, short>>,
              "remove bug");
static_assert(is_same_v<only_ints, tuple<int, short>>, "remove if bug");



/// ituple


TEST(TypeTraits, ITupleTest) {

    // 1.3 should be converted to 1 because it's int
    auto const tup  = ituple<int, double>{1.3, 1.1};
    auto const tup3 = tup.structured<3>();

    auto const [mi_int, mi_double, mi_nothing] = tup3;

    static_assert(is_same_v<int, remove_cvref_t<decltype(mi_int)>>, "it should be int");
    static_assert(is_same_v<double, remove_cvref_t<decltype(mi_double)>>, "it should be double");
    static_assert(is_same_v<nothing_type, remove_cvref_t<decltype(mi_nothing)>>, "it should be nothing");

    EXPECT_EQ(1, get<0>(tup));
    EXPECT_EQ(1.1, get<1>(tup));
    EXPECT_EQ(1, get<0>(tup3));
    EXPECT_EQ(1.1, get<1>(tup3));
    EXPECT_EQ(1, mi_int);
    EXPECT_EQ(1.1, mi_double);
}
