// Created by moisrex on 11/1/19.

#ifndef WEBPP_ROUTES_ROUTE_H
#define WEBPP_ROUTES_ROUTE_H

#include "../../logs/log_concepts.hpp"
#include "../../std/optional.hpp"
#include "../../std/type_traits.hpp"
#include "../../utils/functional.hpp"
#include "./context.hpp"
#include "./route_concepts.hpp"

#include <utility>

namespace webpp::http {


    template <typename Route, typename... Args>
    concept is_callable_route =
      stl::is_invocable_v<Route, stl::remove_cvref_t<Args>...> || stl::is_invocable_v<Route, Args...> ||
      stl::is_invocable_v<Route, stl::add_lvalue_reference_t<stl::remove_cvref_t<Args>>...>;

    template <typename Route, typename... Args>
    concept is_nothrow_callable_route =
      stl::is_nothrow_invocable_v<stl::decay_t<Route>, stl::remove_cvref_t<Args>...> ||
      stl::is_nothrow_invocable_v<stl::decay_t<Route>, Args...> ||
      stl::is_nothrow_invocable_v<stl::decay_t<Route>,
                                  stl::add_lvalue_reference_t<stl::remove_cvref_t<Args>>...>;

    namespace details {

        template <typename CallableT, Context CtxT, typename... Args>
        constexpr auto run_and_catch(CallableT && callable, CtxT & ctx, Args && ... args) noexcept {
            using namespace stl;

            using return_type = invoke_result_t<CallableT, Args...>;
            if constexpr (is_nothrow_invocable_v<CallableT, Args...>) {
                // It's noexcept, we call it knowing that.
                return callable(forward<Args>(args)...);
            } else if constexpr (is_invocable_v<CallableT, Args...>) {

                if constexpr (is_void_v<return_type>) {
                    try {
                        callable(forward<Args>(args)...);
                    } catch (...) {
                        // nothing to do
                    }
                } else if constexpr (same_as<return_type, bool>) {
                    try {
                        return callable(forward<Args>(args)...);
                    } catch (...) { return false; }
                } else if constexpr (istl::Optional<return_type>) {
                    try {
                        return callable(forward<Args>(args)...);
                    } catch (...) {
                        // return 500 error on failure hoping the response type supports it
                        // todo: add more error handling stuff here to the result
                        return typename return_type::value_type{500u};
                    }
                } else {
                    using optional_type = decltype(make_optional(callable(forward<Args>(args)...)));
                    try {
                        return make_optional(callable(forward<Args>(args)...));
                    } catch (...) { return optional_type{nullopt}; }
                }

            } else {
                return ctx.error(
                  500u,
                  invalid_argument("The specified route is not valid. We're not able to call it."));
            }
        }

    } // namespace details


    constexpr auto call_route(auto&& _route, Context auto&& ctx, HTTPRequest auto&& req) noexcept {
        using namespace details;
        using namespace stl;

        using route_type   = stl::remove_cvref_t<decltype(_route)>;
        using request_type = stl::remove_cvref_t<decltype(req)>;
        using ctx_type     = stl::remove_cvref_t<decltype(ctx)>;
        using context_type = stl::add_lvalue_reference_t<ctx_type>;

        if constexpr (is_callable_route<route_type, context_type, request_type>) {
            // requires a context and a request
            if constexpr (!is_void_v<invoke_result_t<route_type, context_type, request_type>>) {
                return run_and_catch(_route, ctx, ctx, req);
            } else {
                run_and_catch(_route, ctx, ctx, req);
            }
        } else if constexpr (is_callable_route<route_type, request_type, context_type>) {
            // requires a request and a context
            if constexpr (!is_void_v<invoke_result_t<route_type, request_type, context_type>>) {
                return run_and_catch(_route, ctx, req, ctx);
            } else {
                run_and_catch(_route, ctx, req, ctx);
            }
        } else if constexpr (is_callable_route<route_type, context_type>) {
            // gets a context
            if constexpr (!is_void_v<invoke_result_t<route_type, context_type>>) {
                return run_and_catch(_route, ctx, ctx);
            } else {
                run_and_catch(_route, ctx, ctx);
            }
        } else if constexpr (is_callable_route<route_type, request_type>) {
            // requires a request
            if constexpr (!is_void_v<invoke_result_t<route_type, request_type>>) {
                return run_and_catch(_route, ctx, req);
            } else {
                run_and_catch(_route, ctx, req);
            }
        } else if constexpr (is_callable_route<route_type>) {
            // requires nothing
            if constexpr (!is_void_v<invoke_result_t<route_type>>) {
                return run_and_catch(_route, ctx);
            } else {
                run_and_catch(_route, ctx);
            }
        } else {
            throw invalid_argument(
              "We don't know how to call your entry route. Change your route's signature.");
        }
    }



    enum class logical_operators { none, AND, OR, XOR };

    template <typename RouteType, logical_operators Op, typename NextRoute>
    struct basic_route : public make_inheritable<RouteType> {
        using next_route_type = NextRoute;
        using super_t         = make_inheritable<RouteType>;

        constexpr static logical_operators op = Op;
        next_route_type                    next;

        //            constexpr basic_route(super_t&&       super = super_t{},
        //                                  next_route_type&& _next = next_route_type{}) noexcept
        //              : super_t(stl::forward<RouteType>(super)),
        //                next(stl::forward<next_route_type>(_next)) {}

        constexpr basic_route(auto&& super, auto&& _next = next_route_type{}) noexcept
          : super_t(stl::forward<decltype(super)>(super)),
            next(stl::forward<decltype(_next)>(_next)) {}

        constexpr basic_route(basic_route const& v) noexcept = default;
        constexpr basic_route(basic_route&& v) noexcept      = default;

        using super_t::operator=;
        using super_t::operator();
    };

    template <typename RouteType>
    struct basic_route<RouteType, logical_operators::none, void> : public make_inheritable<RouteType> {
        using super_t = make_inheritable<RouteType>;

        template <typename... Args>
        constexpr basic_route(Args&&... args) noexcept : super_t{stl::forward<Args>(args)...} {}

        constexpr basic_route(basic_route const&) noexcept = default;
        constexpr basic_route(basic_route&&) noexcept      = default;

        using super_t::operator=;
        using super_t::operator();
    };

    template <>
    struct basic_route<void, logical_operators::none, void> {
        void operator()(Context auto const&) const noexcept {}
    };

    template <typename RouteType     = void,
              logical_operators Op   = logical_operators::none,
              typename NextRouteType = void>
    struct route
      : public basic_route<stl::remove_cvref_t<RouteType>, Op, stl::remove_cvref_t<NextRouteType>> {

        using route_type                      = stl::remove_cvref_t<RouteType>;
        using next_route_type                 = stl::remove_cvref_t<NextRouteType>;
        static constexpr logical_operators op = Op;

      private:
        using super_t = basic_route<route_type, Op, next_route_type>;

        // // todo: use the istl::lazy_conditional
        // template <bool Condition, template <typename...> typename T1, typename T2, typename... C>
        // struct lazy_conditional {};
        //
        // template <template <typename...> typename T1, typename T2, typename... C>
        // struct lazy_conditional<true, T1, T2, C...> {
        //     using type = T1<C...>;
        // };
        //
        // template <template <typename...> typename T1, typename T2, typename... C>
        // struct lazy_conditional<false, T1, T2, C...> {
        //     using type = T2;
        // };
        //
        // template <bool Condition, template <typename...> typename T1, typename T2, typename... C>
        // using lazy_conditional_t = typename lazy_conditional<Condition, T1, T2, C...>::type;
        //
        //
        // template <typename R, typename C, bool IF>
        // struct is_switching_context {
        //     static constexpr bool value = false;
        // };
        //
        // template <typename R, typename C>
        // struct is_switching_context<R, C, true> {
        //     static constexpr bool value = Context<stl::invoke_result_t<R, C>>;
        // };
        //
        // template <typename R, typename C>
        // static constexpr bool is_switching_context_v =
        //   is_switching_context<R, C, !stl::is_void_v<R> && stl::is_invocable_v<R, C>>::value;
        //
        //
        // /**
        //  * Get the "switched context typed" if it's R is a route, otherwise, return the context type if
        //  * the operator(C) of R is doing context switching, otherwise, return the specified default C
        //  * @tparam R
        //  * @tparam C
        //  */
        // template <typename R, typename C>
        // struct lazy_switched_context_type {
        //     using type = lazy_conditional_t<is_switching_context_v<R, C>, stl::invoke_result_t, C, R, C>;
        // };
        //
        // template <typename R, typename C>
        // requires(Route<R, C>) struct lazy_switched_context_type<R, C> {
        //     using type = typename R::template switched_context_type<C>;
        // };

      public:
        constexpr static bool is_route_valid      = !stl::is_void_v<route_type>;
        constexpr static bool is_next_route_valid = !stl::is_void_v<next_route_type>;

        // template <typename R, typename C>
        // using route_switched_context_type =
        //   stl::conditional_t<stl::is_invocable_v<R, C>, typename lazy_switched_context_type<R, C>::type,
        //   C>;
        //
        // template <typename C>
        // using route_context_type = route_switched_context_type<route_type, C>;
        //
        // template <typename C>
        // using next_route_context_type = route_switched_context_type<next_route_type, C>;
        //
        // template <typename C>
        // using switched_context_type = next_route_context_type<route_context_type<C>>;
        //
        // template <typename C>
        // static constexpr bool is_switching_context_recursive = stl::is_same_v<switched_context_type<C>, C>;


        constexpr route() noexcept : super_t{} {}
        constexpr route(route const&) noexcept = default;
        constexpr route(route&&) noexcept      = default;

        template <typename... Args>
        requires(stl::is_constructible_v<super_t, Args...>) constexpr route(Args&&... args) noexcept
          : super_t{stl::forward<Args>(args)...} {}


      private:
        template <logical_operators TheOp, typename NewRouteType>
        [[nodiscard]] constexpr auto set_next(NewRouteType&& new_route) const noexcept {
            if constexpr (stl::is_void_v<next_route_type>) {
                // this part will only execute when the "next_route_type" is void

                // the first way (A<X, void> and B<Y, void> === A<X, B<Y, void>>
                return route<route_type, TheOp, NewRouteType>(*this, stl::forward<NewRouteType>(new_route));
            } else {
                // this means this function has a "next" valve already,
                // so it goes to the next's next valve
                // this way we recursively create a valve type and return it.
                using encapsulated_new_route = route<next_route_type, TheOp, NewRouteType>;
                return route<route_type, op, encapsulated_new_route>{
                  *this,
                  encapsulated_new_route{super_t::next, stl::forward<NewRouteType>(new_route)}};
            }
        }

        template <typename AppType, typename NewRoutePtr, bool is_noexcept>
        struct route_with_router_pointer {
            NewRoutePtr        new_route;
            AppType*           app = nullptr;
            [[nodiscard]] auto operator()(auto... args) noexcept(is_noexcept) {
                // yes we know app must not be nullptr, but route should only be used with router,
                // and the router will set the app if it can otherwise the router can throw an
                // error at compile time or at least at initialization time instead of when the
                // a user's request comes to this specific route. This makes sure the developers
                // will catch this error sooner.
                assert(app != nullptr); // You did not supply the correct app to the router

                return (app->*new_route)(stl::forward<decltype(args)>(args)...);
            }
        };

        [[nodiscard]] constexpr auto call_this_route(Context auto&&     ctx,
                                                     HTTPRequest auto&& req) const noexcept {
            if constexpr (is_route_valid) {
                return call_route(static_cast<super_t>(*this), ctx, req);
            } else {
                return; // void
            }
        }

        [[nodiscard]] constexpr auto call_next_route([[maybe_unused]] Context auto&&     ctx,
                                                     [[maybe_unused]] HTTPRequest auto&& req) const noexcept {
            // using context_type = stl::remove_cvref_t<decltype(ctx)>;
            if constexpr (is_next_route_valid) {
                return call_route(super_t::next, ctx, req);
            } else {
                return; // void
            }
        }

        [[nodiscard]] constexpr bool
        call_next_route_in_bool([[maybe_unused]] Context auto&&     ctx,
                                [[maybe_unused]] HTTPRequest auto&& req) const noexcept {
            using res_type = decltype(call_next_route(stl::forward<decltype(ctx)>(ctx), req));
            using res_t    = stl::remove_cvref_t<res_type>;
            if constexpr (stl::same_as<res_t, bool>) {
                return call_next_route(stl::forward<decltype(ctx)>(ctx), req);
            } else {
                // ignore the results
                (void) call_next_route(stl::forward<decltype(ctx)>(ctx), req);
                return true;
            }
        }

      public:
        [[nodiscard]] constexpr auto operator>>=(void (*func)()) const noexcept {
            return set_next<logical_operators::none>(func);
        }

        template <typename ReturnType>
        [[nodiscard]] constexpr auto operator>>=(ReturnType (*func)()) const noexcept {
            return set_next<logical_operators::none>(func);
        }

        [[nodiscard]] constexpr auto operator>>=(auto&& new_route) const noexcept {
            using rt = stl::remove_cvref_t<decltype(new_route)>;
            if constexpr (stl::is_member_function_pointer_v<rt>) {
                using mem_func_ptr_t = member_function_pointer<rt>;
                using app_type       = typename mem_func_ptr_t::type;
                return set_next<logical_operators::none>(
                  route_with_router_pointer<app_type,
                                            stl::remove_cvref<decltype(new_route)>,
                                            mem_func_ptr_t::is_noexcept>{});
            } else /*if constexpr (PotentialRoute<rt, switched_context_type<fake_context_type>>)*/ {
                return set_next<logical_operators::none>(stl::forward<decltype(new_route)>(new_route));
                // } else {
                //     // todo: write tests for this:
                //     return set_next<logical_operators::none>([=](auto... args) {
                //         //                        static_assert(stl::is_invocable_v<decltype(new_route),
                //         //                        decltype(args)...>,
                //         //                                "The specified route can't be called in any
                //         //                                way that our router knows; " "you might need
                //         //                                to change the signature of your route.");
                //         using nrtype = decltype(new_route);
                //         if constexpr (stl::is_invocable_v<nrtype, decltype(args)...>) {
                //             return new_route(stl::forward<decltype(args)>(args)...);
                //         } else if (stl::is_invocable_v<nrtype>) {
                //             return new_route();
                //         } else {
                //             stl::invalid_argument(
                //               "We're unable to run your route. We don't know how."
                //               " Make sure you're using a route signature that's familiar
                //               for us.");
                //         }
                //     });
            }
        }

#pragma clang diagnostic push
#pragma ide diagnostic   ignored "misc-unconventional-assign-operator"
        [[nodiscard]] constexpr auto operator=(void (*func)()) const noexcept {
            return operator>>=(func);
        }

        template <typename ReturnType>
        [[nodiscard]] constexpr auto operator=(ReturnType (*func)()) const noexcept {
            return operator>>=<ReturnType>(func);
        }

        [[nodiscard]] constexpr auto operator=(auto&& new_route) const noexcept {
            return operator>>=(stl::forward<decltype(new_route)>(new_route));
        }
#pragma clang diagnostic pop


        // template <typename T, typename Ret, typename... Args>
        // [[nodiscard]] constexpr auto operator>>=(Ret (T::*mem_func_pointer)(Args...)) const
        // noexcept {
        //     using app_type = T;
        // }

        // template <typename T, typename Ret, typename... Args>
        // [[nodiscard]] constexpr auto operator=(Ret (T::*mem_func_pointer)(Args...)) const
        // noexcept {
        //     return operator>>=<T, Ret, Args...>(mem_func_pointer);
        // }


        [[nodiscard]] constexpr auto operator&&(/*PotentialRoute*/ auto&& new_route) const noexcept {
            return set_next<logical_operators::AND>(stl::forward<decltype(new_route)>(new_route));
        }

        [[nodiscard]] constexpr auto operator&(/*PotentialRoute*/ auto&& new_route) const noexcept {
            return set_next<logical_operators::AND>(stl::forward<decltype(new_route)>(new_route));
        }

        [[nodiscard]] constexpr auto operator||(/*PotentialRoute*/ auto&& new_route) const noexcept {
            return set_next<logical_operators::OR>(stl::forward<decltype(new_route)>(new_route));
        }

        [[nodiscard]] constexpr auto operator|(/*PotentialRoute*/ auto&& new_route) const noexcept {
            return set_next<logical_operators::OR>(stl::forward<decltype(new_route)>(new_route));
        }

        [[nodiscard]] constexpr auto operator^(/*PotentialRoute*/ auto&& new_route) const noexcept {
            return set_next<logical_operators::XOR>(stl::forward<decltype(new_route)>(new_route));
        }


        constexpr auto operator()(Context auto&& ctx, HTTPRequest auto&& req) const noexcept {
            using namespace stl;
            // exceptions will be handled by the router, unfortunately we're not able to do that here

            using res_t = remove_cvref_t<decltype(call_this_route(ctx, req))>;
            if constexpr (is_void_v<res_t>) {
                call_this_route(ctx, req); // nothing to worry about, it's void
            } else {
                const auto res                     = call_this_route(ctx, req);
                using n_res_t                      = remove_cvref_t<decltype(call_next_route(ctx, req))>;
                constexpr bool convertible_to_bool = is_void_v<n_res_t> || same_as<n_res_t, bool>;

                if constexpr (same_as<res_t, bool>) {
                    // handling sub-route calls:
                    if constexpr (logical_operators::none == op) {
                        // practically the same as AND but without converting the result to boolean
                        if constexpr (is_void_v<n_res_t>) {
                            // just because we can't have an optional<void>
                            if (res)
                                call_next_route(ctx, req);
                        } else {
                            if (res)
                                return optional<n_res_t>{call_next_route(ctx, req)};
                            return optional<n_res_t>{nullopt};
                        }
                    } else if constexpr (logical_operators::AND == op) {
                        // don't rely on operator && for not executing the next route, because the user may
                        // have overloaded the operator &&
                        if constexpr (convertible_to_bool) {
                            if (!res)
                                return true; // continue checking other entry-routes, but not sub-routes
                            return call_next_route_in_bool(ctx, req);
                        } else {
                            if (!res)
                                return optional<n_res_t>{nullopt};
                            return optional<n_res_t>{call_next_route(ctx, req)};
                        }
                    } else if constexpr (logical_operators::OR == op) {
                        // Same as "and", we will not use operator ||
                        if constexpr (convertible_to_bool) {
                            if (res)
                                return true; // continue checking entry-routes but not the sub-routes
                            return call_next_route_in_bool(ctx, req);
                        } else {
                            if (res)
                                return nullopt;
                            return make_optional(call_next_route(ctx, req));
                        }
                    } else if constexpr (logical_operators::XOR == op) {
                        // In operator xor, the next route will be called no matter the result of the current
                        // route so there's no need for doing the same thing that we did above, but since they
                        // may have changed the meaning of the operator ^, it's not a bad idea to do so, but
                        // I'm too lazy :)
                        if constexpr (convertible_to_bool) {
                            return res ^ call_next_route_in_bool(ctx, req);
                        } else {
                            throw invalid_argument("Cannot use xor operator with non-bool route.");
                        }
                    } else {
                        // should not happen ever.
                        return;
                    }
                } else if constexpr (Context<res_t>) {
                    // perform sub-route context switching
                    if constexpr (is_next_route_valid) {
                        return call_next_route(res, req);
                    } else {
                        // entry-route level context-switching will going to happen in the router:
                        return res;
                    }
                } else if constexpr (HTTPResponse<res_t>) {
                    // terminate the sub-route callings and return the response, don't even need to run the
                    // rest of the routes
                    // The strings, and other stuff that can be converted to a response have already been
                    // converted to a response in the "call_route" function.
                    return res;
                } else {
                    // even though the user should not return something useless, and the "call_route" function
                    // takes care of these stuff, we still ignore the result and just run the next routes
                    ctx.logger.warning("Router", "We're ignoring one of your route's results.");
                    return call_next_route(ctx, req);
                }
            }
        }

      private:
        template <typename ResT>
        static void append_route_as_string(istl::String auto& out) {
            using namespace stl;

            // print this route
            if constexpr (is_void_v<ResT>) {
                out.append(" >> action");
            } else if constexpr (same_as<ResT, bool>) {
                // todo: find out the name or type of the validator
                switch (op) {
                    case logical_operators::none: out.append(" >> validator"); break;
                    case logical_operators::AND: out.append(" &&"); break;
                    case logical_operators::OR: out.append(" ||"); break;
                    case logical_operators::XOR: out.append(" ^"); break;
                }
            } else if constexpr (ConvertibleToResponse<ResT>) {
                // todo: find out the type of the response
                out.append(" >> response");
            } else {
                out.append(" >> unknown");
            }
        }

      public:
        /**
         * Generate a string representation of this route
         */
        template <Context CtxT, HTTPRequest ReqT>
        void append_as_string(istl::String auto& out, CtxT&& ctx, ReqT&& req) const {
            using namespace stl;
            using res_t   = remove_cvref_t<decltype(call_this_route(ctx, req))>;
            using n_res_t = remove_cvref_t<decltype(call_next_route(ctx, req))>;

            append_route_as_string<res_t>(out);

            // print the next route
            if constexpr (is_next_route_valid) {
                if constexpr (requires { this->next.template append_as_string<CtxT, ReqT>(out, ctx, req); }) {
                    this->next.template append_as_string<CtxT, ReqT>(out, ctx, req);
                } else {
                    append_route_as_string<n_res_t>(out);
                }
            }
        }

        template <typename StrT = stl::string>
        [[nodiscard]] StrT to_string() const {
            StrT out;
            append_as_string(out);
            return out;
        }
    };


} // namespace webpp::http

#endif // WEBPP_ROUTES_ROUTE_H
