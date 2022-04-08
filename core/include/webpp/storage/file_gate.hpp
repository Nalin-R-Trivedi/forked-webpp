#ifndef WEBPP_STORAGE_FILE_GATE_HPP
#define WEBPP_STORAGE_FILE_GATE_HPP

#include "../traits/default_traits.hpp"

namespace webpp {

    struct file_gate {

        template <Traits TraitsType, CacheKey KeyT, CacheValue ValueT>
        struct storage_gate {
            using key_type    = KeyT;
            using value_type  = ValueT;
            using traits_type = TraitsType;

            template <typename V>
            stl::optional<value_type> get(V&& value) {
                return stl::nullopt;
            }

            template <typename K, typename V>
            void set(K&& key, V&& value) {}

            template <typename K>
            void erase(K&& input) {}

            template <typename Pred>
            void erase_if(Pred&& predicate) {}
        };
    };

} // namespace webpp

#endif // WEBPP_STORAGE_FILE_GATE_HPP
