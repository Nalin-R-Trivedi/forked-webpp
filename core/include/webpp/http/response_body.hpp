#ifndef WEBPP_BODY_H
#define WEBPP_BODY_H

#include "../extensions/extension.hpp"

namespace webpp::http {

    /**
     * There are two types of bodies:
     *   - The request body
     *   - The response body
     *
     * Response Body:
     *   - Features of the response body:
     *     - Should not be a template or we have to implement free functions
     *     - Owns it's data
     *
     *   - Caching system
     *     - In memory and on hard cache:
     *       - [ ] In memory cache with use counts
     *         - so it's possible to migrate the in memory cache into hard disk when
     *           memory is about to finish
     *       - [ ] In hard disk cache (completely remove-able)
     *       - [ ] Permanent cache
     *         - We shouldn't need this. Think about it. TODO
     *     - Static responses:
     *       - [ ] Static files -> html, css, js, images, ...
     *     - Half dynamic responses:
     *       - [ ] The HTML templates
     *     - Full dynamic responses:
     *       - [ ] Forward caching
     *         - Means that the user will predict the user's future request based
     *           on the current and the past requests he/she made and then generates
     *           a response a head of time if the system is not busy.
     *
     *   - Minifying/Compressing the responses:
     *     - [ ] GZip
     *     - [ ] Data themselves:
     *       - [ ] HTML
     *       - [ ] CSS
     *       - [ ] JS
     *       - [ ] Images
     *         - [ ] JPEG
     *         - [ ] GIF
     *         - [ ] PNG
     *
     *   - Writing the data in different formats
     *     - [ ] String
     *     - [ ] JSON from:
     *       - [ ] Strings
     *       - [ ] Iterate-ables: arrays, vectors, maps, ...
     *     - Media
     *       - Images
     *         - [ ] JPEG
     *         - [ ] PNG
     *         - [ ] GIF
     *         - [ ] research more for standards
     *       - Videos
     *         - [ ] mp4
     *         - [ ] flv
     *         - [ ] research more for standards
     *       - Audios
     *         - [ ] mp3
     *         - [ ] research more for standards
     *     - [ ] GraphQL
     *     - [ ] Downloadable files of any type (blobs)
     *
     */
    template <Traits TraitsType, typename EList>
    struct response_body : public EList {
        using traits_type = TraitsType;
        using elist_type  = EList;
        using char_type   = traits::char_type<traits_type>;

        using EList::EList;

        constexpr response_body() requires(stl::is_default_constructible_v<elist_type>) = default;

        // NOLINTBEGIN(bugprone-forwarding-reference-overload)

        template <EnabledTraits ET>
            requires(stl::is_constructible_v<elist_type, ET>)
        constexpr response_body(ET&& et) noexcept(stl::is_nothrow_constructible_v<elist_type, ET>)
          : elist_type{et} {}


        template <EnabledTraits ET>
        constexpr response_body([[maybe_unused]] ET&&) noexcept(
          stl::is_nothrow_default_constructible_v<elist_type>)
          : elist_type{} {}

        // NOLINTEND(bugprone-forwarding-reference-overload)


        [[nodiscard]] constexpr char_type const* data() const noexcept {
            if constexpr (requires(elist_type body) { body.data(); }) {
                return elist_type::data();
            } else {
                return nullptr;
            }
        }

        [[nodiscard]] constexpr stl::streamsize size() const noexcept {
            if constexpr (requires(elist_type body) { body.size(); }) {
                return elist_type::size();
            } else {
                return 0;
            }
        }

        [[nodiscard]] constexpr bool operator==(response_body const& body) const noexcept {
            if constexpr (requires { elist_type::operator==(body); }) {
                return elist_type::operator==(body);
            } else {
                const auto this_size = size();
                return this_size == body.size() && stl::equal(data(), data() + this_size, body.data());
            }
        }


        [[nodiscard]] constexpr bool operator!=(response_body const& body) const noexcept {
            return !operator==(body);
        }
    };


    struct response_body_descriptor {

        template <typename ExtensionType>
        using extractor_type = typename ExtensionType::response_body_extensions;

        template <typename RootExtensions, typename TraitsType, typename EList>
        using mid_level_extensie_type = response_body<TraitsType, EList>;
    };

    template <Traits TraitsType, Extension... E>
    using simple_response_body =
      typename extension_pack<E...>::template extensie_type<TraitsType, response_body_descriptor>;

} // namespace webpp::http

#endif // WEBPP_BODY_H
