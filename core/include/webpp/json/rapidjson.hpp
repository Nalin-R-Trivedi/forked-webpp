// Created by moisrex on 7/3/21.

#ifndef WEBPP_RAPIDJSON_HPP
#define WEBPP_RAPIDJSON_HPP

#if __has_include(<rapidjson/document.h>)
#    define WEBPP_RAPIDJSON_READY
#    include "../memory/buffer.hpp"
#    include "../std/string.hpp"
#    include "../std/string_view.hpp"
#    include "../traits/default_traits.hpp"
#    include "json_concepts.hpp"

#    include <compare>
#    include <cstdio>
#    include <filesystem>

#    define RAPIDJSON_HAS_STDSTRING 1 // enable std::string support for rapidjson (todo: do we need it?)
#    include <rapidjson/document.h>
#    include <rapidjson/filereadstream.h>
#    include <rapidjson/prettywriter.h>

#    if (RAPIDJSON_MAJOR_VERSION == 1 && RAPIDJSON_MINOR_VERSION == 1 && RAPIDJSON_PATCH_VERSION == 0)
namespace rapidjson {
    // a patch for issue: https://github.com/Tencent/rapidjson/pull/1947
    template <bool Const, typename Encoding, typename Allocator>
    GenericMemberIterator<Const, Encoding, Allocator>
    operator+(typename GenericMemberIterator<Const, Encoding, Allocator>::DifferenceType n,
              GenericMemberIterator<Const, Encoding, Allocator> const&                   j) {
        return j + n;
    }

    // This part doesn't need to be sent to rapidjson, it's already there
    // (https://github.com/the-moisrex/rapidjson/blob/f14d5097e51fc19582884a517699adef09edbff7/include/rapidjson/document.h#L262)
    template <bool Const, typename Encoding, typename Allocator>
    bool operator==(GenericMemberIterator<Const, Encoding, Allocator> const& n,
                    GenericMemberIterator<Const, Encoding, Allocator> const& j) {
        return n.operator->() == j.operator->();
    }
} // namespace rapidjson
#    endif

namespace webpp::json::rapidjson {
    // using namespace ::rapidjson;

    /**
     * @brief The goal of this struct is to make this code happen:
     * @code
     *   for (auto [key, value] : doc.as_object()) {
     *       cout << key << ": " << value << endl;
     *   }
     * @endcode
     */
    template <typename MemberValType>
    struct key_value_pair {
        using key_type   = MemberValType;
        using value_type = MemberValType;

        constexpr key_value_pair(key_value_pair const& p) : key{p.key}, value{p.value} {}
        constexpr key_value_pair(key_value_pair&&) noexcept = default;

        constexpr key_value_pair(key_type const& k, value_type const& v) : key{k}, value{v} {}
        constexpr key_value_pair(key_type& k, value_type& v) : key{k}, value{v} {}
        constexpr key_value_pair(key_type&& k, value_type&& v) : key{stl::move(k)}, value{stl::move(v)} {}
        constexpr key_value_pair(key_type const& k, value_type& v) : key{k}, value{v} {}
        constexpr key_value_pair(key_type const& k, value_type&& v) : key{k}, value{stl::move(v)} {}
        constexpr key_value_pair(key_type& k, value_type const& v) : key{k}, value{v} {}
        constexpr key_value_pair(key_type&& k, value_type const& v) : key{stl::move(k)}, value{v} {}

        key_value_pair& operator=(key_value_pair const&) = default;
        key_value_pair& operator=(key_value_pair&&) noexcept = default;

        auto operator<=>(key_value_pair const&) const = default;
        auto operator<=>(MemberValType const& val) const {
            return val <=> value;
        }

        key_type   key;
        value_type value;
    };

    /**
     * todo: add a choice to use rapidjson's allocator
     * todo: use traits_type's allocator correctly if possible
     */
    namespace details {

        template <Traits TraitsType, typename ValueType>
        struct generic_value;

        template <Traits TraitsType, typename ArrayType>
        struct generic_array;

        template <Traits TraitsType, typename ObjectType>
        struct generic_object;

        /**
         * Generic Member Iterator
         */
        template <Traits TraitsType, typename RapidJSONIterator>
        struct generic_member_iterator : public stl::remove_pointer_t<RapidJSONIterator> {

            using traits_type                 = TraitsType;
            using base_type                   = stl::remove_pointer_t<RapidJSONIterator>;
            using rapidjson_reference         = typename base_type::reference;
            using rapidjson_pointer           = typename base_type::pointer;
            using rapidjson_difference_type   = typename base_type::difference_type;
            using rapidjson_value_type        = typename base_type::value_type;
            using rapidjson_iterator_category = typename base_type::iterator_category;
            using rapidjson_const_iterator    = typename base_type::ConstIterator;
            using rapidjson_iterator          = typename base_type::NonConstIterator;
            using rapidjson_member_value_type =
              stl::remove_cvref_t<decltype(stl::declval<stl::remove_cvref_t<rapidjson_value_type>>().name)>;
            using rapidjson_member_value_type_auto =
              stl::conditional_t<stl::is_const_v<rapidjson_value_type>,
                                 stl::add_cv_t<rapidjson_member_value_type>,
                                 stl::add_lvalue_reference_t<rapidjson_member_value_type>>;

            using item_type = generic_value<traits_type, rapidjson_member_value_type_auto>;

            using iterator          = generic_member_iterator;
            using const_iterator    = generic_member_iterator<traits_type, rapidjson_const_iterator> const;
            using iterator_category = rapidjson_iterator_category;
            using value_type        = key_value_pair<item_type>;
            using pointer           = value_type;
            using reference         = value_type;
            using difference_type   = rapidjson_difference_type;

            using base_type::base_type;

            generic_member_iterator(base_type const& iter) : base_type{iter} {}
            generic_member_iterator(base_type&& iter) : base_type{stl::move(iter)} {}
            generic_member_iterator(generic_member_iterator const& iter)     = default;
            generic_member_iterator(generic_member_iterator&& iter) noexcept = default;

            iterator& operator=(iterator&& iter) noexcept = default;
            iterator& operator=(iterator const& iter) noexcept = default;

            //            iterator& operator=(const iterator& iter) {
            //                base_type::opreator = (iter);
            //                return *this;
            //            }



            iterator& operator++() {
                base_type::operator++();
                return *this;
            }
            iterator& operator--() {
                base_type::operator--();
                return *this;
            }

            iterator operator++(int) {
                iterator old(*this);
                this->   operator++();
                return old;
            }
            iterator operator--(int) {
                iterator old(*this);
                this->   operator--();
                return old;
            }


            iterator operator+(difference_type n) const {
                return base_type::operator+(n);
            }
            iterator operator-(difference_type n) const {
                return base_type::operator-(n);
            }

            iterator& operator+=(difference_type n) {
                base_type::operator+=(n);
                return *this;
            }
            iterator& operator-=(difference_type n) {
                base_type::operator-=(n);
                return *this;
            }



            bool operator==(const_iterator that) const {
                return base_type::operator==(that);
            }
            bool operator!=(const_iterator that) const {
                return base_type::operator!=(that);
            }
            bool operator<=(const_iterator that) const {
                return base_type::operator<=(that);
            }
            bool operator>=(const_iterator that) const {
                return base_type::operator>=(that);
            }
            bool operator<(const_iterator that) const {
                return base_type::operator<(that);
            }
            bool operator>(const_iterator that) const {
                return base_type::operator>(that);
            }

            bool operator==(iterator const& that) const {
                return base_type::operator==(that);
            }
            bool operator!=(iterator const& that) const {
                return base_type::operator!=(that);
            }
            bool operator<=(iterator const& that) const {
                return base_type::operator<=(that);
            }
            bool operator>=(iterator const& that) const {
                return base_type::operator>=(that);
            }
            bool operator<(iterator const& that) const {
                return base_type::operator<(that);
            }
            bool operator>(iterator const& that) const {
                return base_type::operator>(that);
            }



            reference operator*() const {
                auto& res = base_type::operator*();
                return {res.name.Move(), res.value.Move()};
            }
            pointer operator->() const {
                return base_type::operator->();
            }
            reference operator[](difference_type n) const {
                return base_type::operator[](n);
            }



            difference_type operator-(const_iterator that) const {
                return base_type::operator-(that);
            }
        };

        template <Traits TraitsType, typename RapidJSONIterator>
        generic_member_iterator<TraitsType, RapidJSONIterator>
        operator+(typename generic_member_iterator<TraitsType, RapidJSONIterator>::diff_t n,
                  generic_member_iterator<TraitsType, RapidJSONIterator> const&           j) {
            return j + n;
        }




        template <Traits TraitsType, typename ValueContainer>
        struct json_common {

          private:
            // DocType could be a document or an GenericObject actually
            template <typename DocType>
            struct value_type_finder {
                using type = typename DocType::ValueType;
            };

          public:
            // finding the rapidjson's ValueType even if the ValueType is a ::rapidjson::Document type or a
            // ::rapidjson::GenericObject
            static constexpr bool has_ref =
              !stl::same_as<ValueContainer, stl::remove_cvref_t<ValueContainer>>;
            using container_type = ValueContainer;
            using value_type     = istl::lazy_conditional_t<
              (requires { typename stl::remove_cvref_t<ValueContainer>::ValueType; }),
              istl::templated_lazy_type<value_type_finder, stl::remove_cvref_t<ValueContainer>>,
              istl::lazy_type<stl::remove_cvref_t<ValueContainer>>>;

            static_assert(
              requires { typename value_type::Object; },
              "The specified ValueType doesn't seem to be a valid rapidjson value");

            using traits_type            = TraitsType;
            using string_type            = traits::general_string<traits_type>;
            using string_view_type       = traits::string_view<traits_type>;
            using char_type              = traits::char_type<traits_type>;
            using generic_value_type     = generic_value<traits_type, value_type>;
            using value_ref              = stl::add_lvalue_reference_t<value_type>; // add & to obj
            using value_const_ref        = stl::add_const_t<value_ref>;             // add const
            using auto_ref_value_type    = stl::conditional_t<has_ref, value_ref, value_type>;
            using value_ref_holder       = generic_value<traits_type, value_ref>;       // ref holder
            using value_const_ref_holder = generic_value<traits_type, value_const_ref>; // ref holder
            using rapidjson_object_type  = typename value_type::Object;
            using object_type            = generic_object<traits_type, rapidjson_object_type>;
            using rapidjson_array_type   = typename value_type::Array;
            using array_type             = generic_array<traits_type, rapidjson_array_type>;

            constexpr json_common()                       = default;
            constexpr json_common(json_common const&)     = default;
            constexpr json_common(json_common&&) noexcept = default;

            json_common& operator=(json_common&&) noexcept = default;
            json_common& operator=(json_common const&) = default;


            template <typename... ValT>
            json_common(ValT&&... obj) : val_handle{stl::forward<ValT>(obj)...} {}

            template <istl::StringViewifiable StrT>
            value_ref_holder operator=(StrT&& val) {
                auto const val_view = istl::string_viewify_of<string_view_type>(stl::forward<StrT>(val));
                val_handle          = ::rapidjson::StringRef(val_view.data(), val_view.size());
                return *this;
            }

            template <typename T>
            value_ref_holder operator=(T&& val) {
                val_handle = stl::forward<T>(val);
                return *this;
            }

            template <typename T>
            [[nodiscard]] bool is() const {
                return val_handle.template Is<T>();
            }


            template <typename T>
            [[nodiscard]] T as() const {
                return val_handle.template Get<T>();
            }

#    define IS_METHOD(real_type, type_name, is_func, get_func, set_func) \
        [[nodiscard]] bool is_##type_name() const {                      \
            return val_handle.is_func();                                 \
        }                                                                \
                                                                         \
        value_ref_holder set_##type_name(real_type const& val) {         \
            val_handle.set_func(val);                                    \
            return *this;                                                \
        }                                                                \
                                                                         \
        value_ref_holder set_##type_name(real_type&& val) {              \
            val_handle.set_func(stl::move(val));                         \
            return *this;                                                \
        }                                                                \
                                                                         \
        [[nodiscard]] real_type as_##type_name() const {                 \
            return val_handle.get_func();                                \
        }


#    define WEBPP_IS_OPERATOR(real_type, type_name) \
        [[nodiscard]] operator real_type() const {  \
            return as_##type_name();                \
        }

            // WEBPP_IS_METHOD(null, IsNull, SetNull)
            IS_METHOD(bool, bool, IsBool, GetBool, SetBool)
            IS_METHOD(stl::int8_t, int8, IsInt, GetInt, SetInt)
            IS_METHOD(stl::int16_t, int16, IsInt, GetInt, SetInt)
            IS_METHOD(stl::int32_t, int32, IsInt, GetInt, SetInt)
            IS_METHOD(stl::int64_t, int64, IsInt, GetInt, SetInt)
            IS_METHOD(double, double, IsDouble, GetDouble, SetDouble)
            IS_METHOD(float, float, IsFloat, GetFloat, SetFloat)
            IS_METHOD(stl::uint8_t, uint8, IsUint, GetUint, SetUint)
            IS_METHOD(stl::uint16_t, uint16, IsUint, GetUint, SetUint)
            IS_METHOD(stl::uint32_t, uint32, IsUint, GetUint, SetUint)
            IS_METHOD(stl::uint64_t, uint64, IsUint64, GetUint64, SetUint64)
            // WEBPP_IS_METHOD(stl::string, string, IsString, GetString, SetString)

            WEBPP_IS_OPERATOR(bool, bool)
            WEBPP_IS_OPERATOR(stl::int8_t, int8)
            WEBPP_IS_OPERATOR(stl::int16_t, int16)
            WEBPP_IS_OPERATOR(stl::int32_t, int32)
            WEBPP_IS_OPERATOR(stl::int64_t, int64)
            WEBPP_IS_OPERATOR(stl::uint8_t, uint8)
            WEBPP_IS_OPERATOR(stl::uint16_t, uint16)
            WEBPP_IS_OPERATOR(stl::uint32_t, uint32)
            WEBPP_IS_OPERATOR(stl::uint64_t, uint64)
            WEBPP_IS_OPERATOR(double, double)
            WEBPP_IS_OPERATOR(float, float)
            WEBPP_IS_OPERATOR(string_type, string)
            WEBPP_IS_OPERATOR(string_view_type, string_view)

            // set here has no values!
            // todo: make custom Array and Object structs and don't rely on rapidjson
            // WEBPP_IS_METHOD(rapidjson_value_type::Array, array, IsArray, GetArray, SetArray)
            // WEBPP_IS_METHOD(rapidjson_value_type::Object, object, IsObject, GetObject, SetObject)
            // WEBPP_IS_METHOD(number, IsNumber, GetNumber, SetNumber)
            // WEBPP_IS_METHOD(true, IsTrue)
            // WEBPP_IS_METHOD(false, IsFalse)

#    undef IS_METHOD
#    undef WEBPP_IS_OPERATOR

#    define RENAME(ret_type, orig_name, new_name, details) \
        ret_type new_name() details {                      \
            return val_handle.orig_name();                 \
        }

            RENAME(object_type, GetObject, as_object, );
            RENAME(array_type, GetArray, as_array, );
            RENAME(bool, IsNull, is_null, const);
            RENAME(bool, IsString, is_string, const);
            RENAME(bool, IsObject, is_object, const);
            RENAME(bool, IsArray, is_array, const);

#    undef RENAME


            template <istl::String StrT = string_type, typename... Args>
            StrT as_string(Args&&... string_args) const {
                if constexpr (sizeof...(Args) == 0) {
                    return StrT{val_handle.GetString(), val_handle.GetStringLength()};
                } else {
                    StrT output{stl::forward<Args>(string_args)...};
                    output.append(val_handle.GetString(), val_handle.GetStringLength());
                    return output;
                }
            }

            string_view_type as_string_view() const {
                return string_view_type{val_handle.GetString(), val_handle.GetStringLength()};
            }

            value_ref_holder set_string(string_view_type str) {
                // todo: use allocator if possible
                val_handle.SetString(str.data(), str.size());
                return *this;
            }

            template <istl::String StrT = string_type, typename... Args>
            StrT pretty(Args&&... string_args) const {
                StrT output{stl::forward<Args>(string_args)...};
                ::rapidjson::PrettyWriter<stl::istringstream>{output};
                return output;
            }

            template <istl::String StrT = string_type, typename... Args>
            StrT uglified(Args&&... string_args) const {
                return as_string(stl::forward<Args>(string_args)...);
            }


          protected:
            container_type val_handle{};
        };




        /**
         * This is a generic array holder
         */
        template <Traits TraitsType, typename ArrayType>
        struct generic_array {
            using traits_type          = TraitsType;
            using rapidjson_array_type = ArrayType;

            generic_array(rapidjson_array_type& arr) : arr_handle{arr} {}

            [[nodiscard]] stl::size_t size() const {
                return arr_handle.Size();
            }

            [[nodiscard]] stl::size_t capacity() const {
                return arr_handle.Capacity();
            }


            auto begin() const {
                return arr_handle.Begin();
            }
            auto end() const {
                return arr_handle.End();
            }

            auto cbegin() const {
                return arr_handle.Begin();
            }
            auto cend() const {
                return arr_handle.End();
            }



          protected:
            rapidjson_array_type& arr_handle;
        };

        /**
         * Generic number will hold
         *   - json numeric values
         *   - booleans
         * It hold booleans too because it's not JavaScript and bools are still numbers! :)
         */
        template <Traits TraitsType, typename ValueType>
        struct generic_number : public json_common<TraitsType, ValueType> {
            using rapidjson_value_type = ValueType;
            using traits_type          = TraitsType;
            using value_type           = generic_value<traits_type, rapidjson_value_type>;
            using string_view_type     = traits::string_view<traits_type>;
            using json_common_type     = json_common<TraitsType, ValueType>;

            using json_common_type::json_common; // common ctors

            template <typename T>
            requires(stl::is_arithmetic_v<T>) // only numbers
              generic_number(T val)
              : json_common_type{rapidjson_value_type{val}} {}

            /**
             * Checks whether a number can be losslessly converted to a float.
             */
            [[nodiscard]] bool is_lossless_float() const {
                return this->val_handle.IsLosslessFloat();
            }
        };

        /**
         * This is a json object which means it can hold a key/value pair of value objects.
         * The ValueType is a rapidjson value type not a generic value type.
         */
        template <Traits TraitsType, typename ObjectType>
        struct generic_object {

            static_assert(istl::is_valued_specialization_of_v<ObjectType, ::rapidjson::GenericObject>,
                          "it's an object not a value");

            using rapidjson_object_type           = ObjectType;
            using rapidjson_plain_value_type      = typename rapidjson_object_type::PlainType;
            using traits_type                     = TraitsType;
            using value_type                      = generic_value<traits_type, rapidjson_plain_value_type>;
            using string_view_type                = traits::string_view<traits_type>;
            using rapidjson_member_iterator       = typename rapidjson_object_type::MemberIterator;
            using rapidjson_const_member_iterator = typename rapidjson_object_type::ConstMemberIterator;
            using iterator_type       = generic_member_iterator<traits_type, rapidjson_member_iterator>;
            using const_iterator_type = generic_member_iterator<traits_type, rapidjson_const_member_iterator>;

            // todo: add more optimization for reference and const reference and move
            // rapidjson_object_type might be a reference itself.
            generic_object(rapidjson_object_type obj) : obj_handle{obj} {}


            template <JSONKey KeyType>
            [[nodiscard]] value_type operator[](KeyType&& key) {
                if constexpr (JSONNumber<KeyType>) {
                    // fixme: write tests for this, this will run the "index" version, right?
                    return obj_handle.operator[](key);
                } else if constexpr (JSONString<KeyType>) {
                    // The key is convertible to string_view
                    auto const key_view =
                      istl::string_viewify_of<string_view_type>(stl::forward<KeyType>(key));
                    return obj_handle[rapidjson_plain_value_type{
                      ::rapidjson::StringRef(key_view.data(), key_view.size())}];
                }
            }

            generic_object& clear() {
                obj_handle.Clear();
                return *this;
            }

            [[nodiscard]] stl::size_t size() const noexcept {
                return obj_handle.MemberCount();
            }

            template <JSONKey KeyT, PotentialJSONValue ValT>
            generic_object& insert(KeyT&& key, ValT&& val) {
                auto const key_view = istl::string_viewify_of<string_view_type>(stl::forward<KeyT>(key));
                obj_handle.AddMember(::rapidjson::StringRef(key_view.data(), key_view.size()),
                                     stl::forward<ValT>(val));
                return *this;
            }

            template <JSONKey KeyT, PotentialJSONValue ValT>
            generic_object& emplace(KeyT&& key, ValT&& val) {
                return insert<KeyT, ValT>(stl::forward<KeyT>(key), stl::forward<ValT>(val));
            }

            static_assert(stl::random_access_iterator<typename rapidjson_object_type::MemberIterator>);
            iterator_type begin() const {
                return obj_handle.MemberBegin();
            }
            iterator_type end() const {
                return obj_handle.MemberEnd();
            }

            iterator_type cbegin() const {
                return obj_handle.MemberBegin();
            }
            iterator_type cend() const {
                return obj_handle.MemberEnd();
            }

            template <JSONKey KeyT>
            [[nodiscard]] bool contains(KeyT&& key) const {
                const auto key_view = istl::string_viewify_of<string_view_type>(stl::forward<KeyT>(key));
                return obj_handle.HasMember(key_view.data()); // fixme: not passing the length
            }


          protected:
            rapidjson_object_type obj_handle;
        };

        template <Traits TraitsType, typename ValueType>
        struct generic_value : public json_common<TraitsType, ValueType> {
            using traits_type            = TraitsType;
            using common_type            = json_common<traits_type, ValueType>;
            using string_type            = typename common_type::string_type;
            using string_view_type       = typename common_type::string_view_type;
            using char_type              = typename common_type::char_type;
            using generic_value_type     = typename common_type::generic_value_type;
            using value_ref              = typename common_type::value_ref;
            using value_ref_holder       = typename common_type::value_ref_holder;
            using value_const_ref_holder = typename common_type::value_const_ref_holder;
            using rapidjson_object_type  = typename common_type::rapidjson_object_type;
            using object_type            = typename common_type::object_type;
            using rapidjson_array_type   = typename common_type::rapidjson_array_type;
            using array_type             = typename common_type::array_type;
            using value_type             = typename common_type::value_type;


            using json_common<TraitsType, ValueType>::json_common;
            using json_common<TraitsType, ValueType>::operator=;

            /**
             * Check if it has a member
             */
            [[nodiscard]] bool contains(string_view_type key) const {
                return this->val_handle.HasMember(::rapidjson::StringRef(key.data(), key.size()));
            }


            template <typename T>
            [[nodiscard]] auto operator[](T&& val) {
                using res_type = decltype(this->val_handle[stl::forward<T>(val)]);
                if constexpr (stl::same_as<res_type, stl::remove_cvref_t<res_type>>) {
                    return generic_value_type{this->val_handle[stl::forward<T>(val)]};
                } else {
                    return value_ref_holder{this->val_handle[stl::forward<T>(val)]};
                }
            }

            template <typename T>
            [[nodiscard]] value_const_ref_holder operator[](T&& val) const {
                return {this->val_handle[stl::forward<T>(val)]};
            }


            //            template <stl::size_t N>
            //            [[nodiscard]] auto operator[](char_type const child_name[N]) {
            //                return
            //                obj_handle.operator[](::rapidjson::GenericStringRef<char_type>(child_name,
            //                N));
            //            }

            //            template <stl::size_t N>
            //            [[nodiscard]] auto operator[](char_type const child_name[N]) const {
            //                using value_type =
            //                  stl::remove_cvref_t<decltype(rapidjson_value_type::operator[](child_name))>;
            //                using new_value_type = stl::add_cv_t<general_value<traits_type, value_type>>;
            //                return new_value_type{rapidjson_value_type::operator[](StringRef(child_name))};
            //            }

#    define RENAME(ret_type, orig_name, new_name, details) \
        ret_type new_name() details {                      \
            return this->val_handle.orig_name();           \
        }

            RENAME(stl::size_t, Size, size, const);
            RENAME(bool, Empty, empty, const);
            RENAME(stl::size_t, Capacity, capacity, const);
            RENAME(void, Clear, clear, );

            //            RENAME(generic_iterator_type, Begin, begin, );
            //            RENAME(generic_iterator_type, Begin, begin, const);
            //            RENAME(generic_iterator_type, End, end, );
            //            RENAME(generic_iterator_type, End, end, const);
            //            RENAME(generic_iterator_type, Begin, cbegin, const);
            //            RENAME(generic_iterator_type, End, cend, const);

#    undef RENAME

            // this is a nice idea, but we have to have common iterator wrapper for this to work
            /*
#    define webpp_iterator_method(iter_name, constness)                                               \
                                                                                                      \
        auto iter_name() constness {                                                                  \
            if (this->is_array()) {                                                                   \
                return this->as_array().iter_name();                                                  \
            } else if (this->is_object()) {                                                           \
                return this->as_object().iter_name();                                                 \
            } else if (this->is_string()) {                                                           \
                return this->as_string_view().iter_name();                                            \
            } else {                                                                                  \
                throw stl::invalid_argument("This json value is not an array, object,"                \
                                            " or a string so you cannot get an iterator out of it."); \
            }                                                                                         \
        }

            webpp_iterator_method(begin, )         // begin
              webpp_iterator_method(end, )         // end
              webpp_iterator_method(cbegin, const) // cbegin
              webpp_iterator_method(cend, const)   // cend

#    undef webpp_iterator_method
            */
        };



    } // namespace details

    template <Traits TraitsType = default_traits>
    using value = details::generic_value<TraitsType, ::rapidjson::Value>;

    template <Traits TraitsType = default_traits>
    struct document : public details::generic_value<TraitsType, ::rapidjson::Document> {
        using traits_type              = TraitsType;
        using string_view_type         = traits::string_view<traits_type>;
        using char_type                = traits::char_type<traits_type>;
        using general_allocator_type   = traits::general_allocator<traits_type, char_type>;
        using value_type               = value<traits_type>;
        using rapidjson_document_type  = ::rapidjson::Document;
        using rapidjson_value_type     = typename rapidjson_document_type::ValueType;
        using rapidjson_allocator_type = typename rapidjson_document_type::AllocatorType;
        using object_type = details::generic_object<traits_type, typename rapidjson_value_type::Object>;
        using array_type  = details::generic_array<traits_type, typename rapidjson_value_type::Array>;
        using generic_value_type = details::generic_value<traits_type, rapidjson_document_type>;

        /**
         * A document containing null
         */
        document() = default;

        /**
         * Get the file and parse it.
         */
        explicit document(stl::filesystem::path file_path) {
            stl::FILE* fp = stl::fopen(file_path.c_str(), "rb");

            stack<65536>                read_buffer;
            ::rapidjson::FileReadStream is(fp, read_buffer.data(), read_buffer.size());
            this->ParseStream(is);

            stl::fclose(fp);
        }

        /**
         * Parse the json string specified here
         */
        template <istl::StringViewifiable StrT>
        requires(!stl::same_as<stl::remove_cvref_t<StrT>, document>) // not a copy/move ctor
          document(StrT&& json_string)
          : generic_value_type{} {
            parse(stl::forward<StrT>(json_string));
        }

        /**
         * A document containing the specified, already parsed, value
         */
        template <typename ConvertibleToValue>
        requires(!istl::StringViewifiable<ConvertibleToValue> &&
                 (stl::convertible_to<stl::remove_cvref_t<ConvertibleToValue>,
                                      value_type> && // check if it's a value or an object
                  !stl::same_as<stl::remove_cvref_t<stl::remove_cvref_t<ConvertibleToValue>>, document>) )
          document(ConvertibleToValue&& val)
          : generic_value_type{stl::forward<ConvertibleToValue>(val)} {}


        template <typename T>
        document& operator=(T&& val) {
            static_cast<generic_value_type*>(this)->operator=(stl::forward<T>(val));
            return *this;
        }


        // implement the parse method
        template <istl::StringViewifiable StrT>
        document& parse(StrT&& json_string) {
            const auto json_str_view = istl::string_viewify(stl::forward<StrT>(json_string));
            this->val_handle.Parse(json_str_view.data(), json_str_view.size());
            return *this;
        }
    };

} // namespace webpp::json::rapidjson

#endif

#endif // WEBPP_RAPIDJSON_HPP
