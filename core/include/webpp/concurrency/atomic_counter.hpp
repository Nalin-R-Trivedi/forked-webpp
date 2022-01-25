// Created by moisrex on 10/26/20.

#ifndef WEBPP_ATOMIC_COUNTER_HPP
#define WEBPP_ATOMIC_COUNTER_HPP

#include "../std/concepts.hpp"

#include <atomic>
#include <compare>

namespace webpp {

    template <typename T = stl::size_t>
    struct atomic_counter {
        stl::atomic<T> counter;

        inline void up() noexcept {
            counter.fetch_add(1, std::memory_order_relaxed);
        }

        inline bool down() noexcept {
            if (counter.fetch_sub(1, std::memory_order_release) == 1) {
                std::atomic_thread_fence(std::memory_order_acquire);
                return true;
            }
            return false;
        }

        atomic_counter& operator++() noexcept {
            up();
            return *this;
        }

        atomic_counter& operator--() noexcept {
            counter.fetch_sub(1, std::memory_order_relaxed);
            return *this;
        }

        constexpr bool operator==(stl::integral auto value) const noexcept {
            return counter == static_cast<T>(value);
        }


        constexpr auto operator<=>(stl::integral auto value) const noexcept {
            return counter <=> static_cast<T>(value);
        }
    };

} // namespace webpp

#endif // WEBPP_ATOMIC_COUNTER_HPP
