#ifndef WEBPP_VALVE_URI_H
#define WEBPP_VALVE_URI_H

#include "../../../include/webpp/common/meta.h"
#include "../../../include/webpp/utils/uri.h"
#include "valve.h"

#include <cassert>
#include <cstddef> // for std::size_t
#include <string_view>

namespace webpp::valves {

    struct path_condition {
      protected:
        const_uri _path;

      public:
        constexpr path_condition(std::string_view str) noexcept
          : _path(std::move(str)) {
        }

        constexpr path_condition() noexcept = default;

        template <typename RequestType>
        [[nodiscard]] bool operator()(RequestType const& req) const noexcept {
            return equal_path(req.request_uri(), _path);
        }
    };

    struct path : public valve<path_condition> {
        using valve<path_condition>::valve;
    };

    constexpr path operator""_path(const char* str, std::size_t len) {
        return path{std::string_view{str, len}};
    }

    /**
     * Check whether or not the specified URI path is a match for the specified
     * template. This function will be used in "tpath_condition". I didn't
     * implement it there because it's a template method and I'd like to
     * abstract away the implementation details.
     * @param templated_path
     * @param _path
     * @return
     */
    template <typename CharT>
    [[nodiscard]] bool
    tpath_check(std::basic_string_view<CharT> const& templated_path,
                std::basic_string_view<CharT> const& _path) noexcept {
        auto          tit        = templated_path.data(); // templated iterator
        auto          pit        = _path.data();          // path iterator
        decltype(tit) seg_start  = nullptr;
        std::size_t   seg_starts = 0;
        decltype(pit) pseg_start = nullptr;
        for (;;) {
            switch (*tit) {
                case '\0':

                    // error: there's an open segment rule and we can't just
                    // leave it unclosed. I should throw an error in the user's
                    // face but I'm nice and I'm just gonna assert
                    assert(("the specified templated path is not valid.",
                            seg_start == nullptr));
                    if (seg_start) {
                        return false;
                    }
                    break;
                case '{': // might be start of a segment

                    if (seg_start) {
                        // the { char is part of the segment rule here
                        ++seg_starts;
                        break;
                    }

                    // pinpoint the segment start for when we reach the end of
                    // it
                    seg_start  = tit + 1;
                    pseg_start = pit;
                    break;
                case '}':
                    { // might be the end of a segment

                        // a new } char with no starting { char
                        assert(("The specified templated path is not valid.",
                                seg_starts != 0));
                        if (seg_starts == 0)
                            return false;

                        --seg_starts;
                        if (seg_starts != 0)
                            break; // this } char is part of the segment rule
                                   // itself

#if CXX20
                        std::basic_string_view<CharT> seg_rule(seg_start, tit);
#elif CXX17
                        std::basic_string_view<CharT> seg_rule(seg_start,
                                                               tit - seg_start);
#endif

                        // going to find the segment in the path string too
                        for (auto it = pit;; ++it) {
                            switch (*it) {
                                case '\0': goto after_loop;
                                case '/':
                            }
                        }
                    after_loop:
                        parse_seg(seg_rule, );
                    }
                    break;
                case '\\': // escape character
                    // we just don't care what the next char is. even if it's {
                    // or } which they have meanings in this algorithm.
                    ++tit;
                    if (*tit != '}' && *tit != '{')
                        --tit;
                    break;
                default:
                    // it's not an error, it's normal to return false here.
                    if (!seg_start && *tit != *pit)
                        return false;
            }

            ++tit;
            pit += seg_start != nullptr; // only add 1 if we're not in a segment
        }
        return false;
    }

    struct tpath_condition {
      protected:
        std::string_view tpath_str;

      public:
        constexpr tpath_condition(std::string_view str) noexcept
          : tpath_str(str) {
        }

        constexpr tpath_condition() noexcept = default;

        template <typename RequestType>
        [[nodiscard]] inline bool
        operator()(RequestType const& req) const noexcept {
            return tpath_check(tpath_str, req.request_uri());
        }
    };

    /**
     * Features:
     *   - [ ] Specifying the type of the segment
     *   - [ ] Validating the segments with a custom method
     *   - [ ] Partial segments: segments that are not between two slashes
     *   - [ ] Naming the segments
     *   - [ ] Variadic segments: segments that contain multiple path segments
     *   - [ ] Default value for segments
     *     - [ ] string as default value
     *     - [ ] integer types as default value
     *     - [ ] custom type as a default value
     *     - [ ] custom object as default value
     *   - [ ] Making a segment optional
     *   - [ ] Custom SegTypes (Segment Types):
     *     - [ ]
     * Examples of tpath:
     *   - /{@int:user_id}/profile
     *   - /{@username:username}/profile
     *   - /{@int}
     *   - /{@email}
     *   - /page/{@uint:page_num}.html
     *   - /product/{@product_list:prod_name}/view
     *   - /view/{view_name}
     *   - /{one}/{two}
     *   - /{slugs...}/page/{@uint:page_num}
     *   - /{controller=Home}/{action=Index}/{id?}
     * Attention: getting those segments are the responsibility of the
     * "route" class. We will define the implementation for it here, but the
     * final user should get the data from there; they can use this feature
     * directly here, but it looks nicer if they do it there.
     */
    struct tpath : public valve<tpath_condition> {
        using valve<tpath_condition>::valve;
    };

    constexpr tpath operator""_tpath(const char* str, std::size_t len) {
        return tpath{std::string_view{str, len}};
    }

} // namespace webpp::valves

#endif // WEBPP_VALVE_URI_H
