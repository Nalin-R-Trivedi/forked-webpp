// Created by moisrex on 10/8/20.

#ifndef WEBPP_STRING_TOKENIZER_HPP
#define WEBPP_STRING_TOKENIZER_HPP

#include "../std/cassert.hpp"
#include "../std/string_view.hpp"
#include "charset.hpp"

namespace webpp {

    enum struct string_tokenizer_options : stl::uint_fast8_t {
        // Specifies the delimiters should be returned as tokens
        return_delims = 1u << 0u,

        // Specifies that empty tokens should be returned. Treats the beginning and
        // ending of the string as implicit delimiters, though doesn't return them
        // as tokens if "return_delims" is also used.
        return_empty_tokens = 1u << 1u,
    };

    /*
     * basic_string_tokenizer is a simple string tokenizer class.  It works like an
     * iterator that with each step (see the Advance method) updates members that
     * refer to the next token in the input string.  The user may optionally
     * configure the tokenizer to return delimiters.
     *
     * EXAMPLE 1:
     *
     *   char input[] = "this is a test";
     *   c_string_tokenizer t(input, input + strlen(input), " ");
     *   while (t.GetNext()) {
     *     printf("%s\n", t.token().c_str());
     *   }
     *
     * Output:
     *
     *   this
     *   is
     *   a
     *   test
     *
     *
     * EXAMPLE 2:
     *
     *   std::string input = "no-cache=\"foo, bar\", private";
     *   string_tokenizer t(input, ", ");
     *   t.set_quote_chars("\"");
     *   while (t.GetNext()) {
     *     printf("%s\n", t.token().c_str());
     *   }
     *
     * Output:
     *
     *   no-cache="foo, bar"
     *   private
     *
     *
     * EXAMPLE 3:
     *
     *   bool next_is_option = false, next_is_value = false;
     *   std::string input = "text/html; charset=UTF-8; foo=bar";
     *   string_tokenizer<> t(input);
     *   while (t.next<"; =", {}, string_tokenizer_options::return_delims>()) {
     *     if (t.token_is_delim()) {
     *       switch (*t.token_begin()) {
     *         case ';':
     *           next_is_option = true;
     *           break;
     *         case '=':
     *           next_is_value = true;
     *           break;
     *       }
     *     } else {
     *       const char* label;
     *       if (next_is_option) {
     *         label = "option-name";
     *         next_is_option = false;
     *       } else if (next_is_value) {
     *         label = "option-value";
     *         next_is_value = false;
     *       } else {
     *         label = "mime-type";
     *       }
     *       printf("%s: %s\n", label, t.token().c_str());
     *     }
     *   }
     *
     */
    template <typename StringViewType = stl::string_view,
              typename ConstIterType  = typename StringViewType::const_iterator>
    class string_tokenizer {
      public:
        using str_v          = StringViewType;
        using const_iterator = ConstIterType;
        using char_type      = typename str_v::value_type;

        static constexpr bool is_raw_pointer_iterator = stl::is_pointer_v<const_iterator>;


        constexpr void init(const_iterator string_begin, const_iterator string_end) {
            _start_pos   = string_begin;
            _token_begin = string_begin;
            _token_end   = string_begin;
            _end         = string_end;
            _is_delim    = true;
        }

        constexpr string_tokenizer(str_v str) noexcept
          : _start_pos{str.begin()},
            _token_begin{str.begin()},
            _token_end{str.begin()},
            _end{str.end()} {}

        constexpr string_tokenizer(const_iterator str_begin, const_iterator str_end) noexcept
          : _start_pos{str_begin},
            _token_begin{str_begin},
            _token_end{str_begin},
            _end{str_end} {}

        // Set the characters to regard as quotes.  By default, this is empty.  When
        // a quote char is encountered, the tokenizer will switch into a mode where
        // it ignores delimiters that it finds.  It switches out of this mode once it
        // finds another instance of the quote char.  If a backslash is encountered
        // within a quoted string, then the next character is skipped.




        // Call this method to advance the tokenizer to the next delimiter.  This
        // returns false if the tokenizer is complete.  This method must be called
        // before calling any of the token* methods.
        template <CharSet auto Delims  = charset<typename StringViewType::value_type, 0>{},
                  CharSet auto Quotes  = charset<typename StringViewType::value_type, 1>{'\0'},
                  int          Options = 0>
        [[nodiscard("Don't skip the value of this, you need a while loop")]] constexpr bool next() noexcept {
            if constexpr (Options == 0 && Quotes.empty()) {
                return quick_get_next<Delims>();
            } else {
                return full_get_next<Delims, Quotes, Options>();
            }
        }

        template <CharSet auto Chars>
        constexpr bool next_until() noexcept {
            _token_begin = _token_end;
            while (_token_end != _end && Chars.contains(*_token_end))
                ++_token_end;
            return true;
        }

        template <CharSet auto Chars>
        constexpr bool next_until_not() noexcept {
            _token_begin = _token_end;
            while (_token_end != _end && !Chars.contains(*_token_end))
                ++_token_end;
            return true;
        }

        template <CharSet auto Chars>
        constexpr void skip() noexcept {
            while (_token_begin != _end && Chars.contains(*_token_begin))
                ++_token_begin;
            _token_end = _token_begin;
        }

        constexpr void skip_spaces() noexcept {
            skip<charset<char_type, 1>(' ')>();
        }


        // Start iterating through tokens from the beginning of the string.
        constexpr void reset() noexcept {
            _token_end = _start_pos;
        }

        // Returns true if token is a delimiter.  When the tokenizer is constructed
        // with the return_relims option, this method can be used to check if the
        // returned token is actually a delimiter. Returns true before the first
        // time get_next() has been called, and after get_next() returns false.
        [[nodiscard]] constexpr bool token_is_delim() const noexcept {
            return _is_delim;
        }

        // If get_next() returned true, then these methods may be used to read the
        // value of the token.
        [[nodiscard]] constexpr const_iterator token_begin() const noexcept {
            return _token_begin;
        }

        [[nodiscard]] constexpr const_iterator token_end() const noexcept {
            return _token_end;
        }

        [[nodiscard]] constexpr str_v token() const noexcept {
            if constexpr (is_raw_pointer_iterator) {
                return str_v(_token_begin, static_cast<stl::size_t>(_token_end - _token_begin));
            } else {
                return str_v(_token_begin, _token_end);
            }
        }

        constexpr void skip_token() noexcept {
            _token_begin = _token_end;
        }

        //        [[nodiscard]] string_view_type token_view() const noexcept {
        //            return string_view_type{&*_token_begin, std::distance(_token_begin, _token_end)};
        //        }

      private:
        // Implementation of get_next() for when we have no quote characters. We have
        // two separate implementations because advance_one() is a hot spot in large
        // text files with large tokens.
        template <CharSet auto Delims>
        constexpr bool quick_get_next() noexcept {
            _is_delim = false;
            for (;;) {
                _token_begin = _token_end;
                if (_token_end == _end) {
                    _is_delim = true;
                    return false;
                }
                ++_token_end;
                if (!Delims.contains(*_token_begin))
                    break;
                // else skip over delimiter.
            }
            while (_token_end != _end && !Delims.contains(*_token_end))
                ++_token_end;
            return true;
        }

        // Implementation of get_next() for when we have to take quotes into account.
        template <CharSet auto Delims, CharSet auto Quotes, int Options>
        constexpr bool full_get_next() noexcept {
            advance_state state;

            for (;;) {
                if (_is_delim) {
                    // Last token was a delimiter. Note: This is also the case at the start.
                    //
                    //    ... D T T T T D ...
                    //        ^ ^
                    //        | |
                    //        | |token_end_| : The next character to look at or |end_|.
                    //        |
                    //        |token_begin_| : Points to delimiter or |token_end_|.
                    //
                    // The next token is always a non-delimiting token. It could be empty,
                    // however.
                    _is_delim    = false;
                    _token_begin = _token_end;

                    // Slurp all non-delimiter characters into the token.
                    while (_token_end != _end && advance_one<Delims, Quotes>(&state, *_token_end)) {
                        ++_token_end;
                    }

                    // If it's non-empty, or empty tokens were requested, return the token.
                    if constexpr (Options & static_cast<int>(string_tokenizer_options::return_empty_tokens)) {
                        return true; // NOLINT(bugprone-branch-clone)
                    } else if (_token_begin != _token_end) {
                        return true;
                    }
                }

                assert(!_is_delim);
                // Last token was a regular token.
                //
                //    ... T T T D T T ...
                //        ^     ^
                //        |     |
                //        |     token_end_ : The next character to look at. Always one
                //        |                  char beyond the token boundary.
                //        |
                //        token_begin_ : Points to beginning of token. Note: token could
                //                       be empty, in which case
                //                       token_begin_ == token_end_.
                //
                // The next token is always a delimiter. It could be |end_| however, but
                // |end_| is also an implicit delimiter.
                _is_delim    = true;
                _token_begin = _token_end;

                if (_token_end == _end)
                    return false;

                // Look at the delimiter.
                ++_token_end;
                if constexpr (Options & static_cast<int>(string_tokenizer_options::return_delims))
                    return true;
            }
        }

        struct advance_state {
            char_type quote_char = '\0';
            bool      in_quote   = false;
            bool      in_escape  = false;
        };

        // Returns true if a delimiter was not hit.
        template <CharSet auto Delims, CharSet auto Quotes>
        static constexpr bool advance_one(advance_state* state, char_type c) noexcept {
            if (state->in_quote) {
                if (state->in_escape) {
                    state->in_escape = false;
                } else if (c == '\\') {
                    state->in_escape = true;
                } else if (c == state->quote_char) {
                    state->in_quote = false;
                }
            } else {
                if (Delims.contains(c))
                    return false;
                state->in_quote = Quotes.contains(state->quote_char = c);
            }
            return true;
        }

        const_iterator _start_pos;
        const_iterator _token_begin;
        const_iterator _token_end;
        const_iterator _end;
        bool           _is_delim = true;
    };

} // namespace webpp

#endif // WEBPP_STRING_TOKENIZER_HPP
