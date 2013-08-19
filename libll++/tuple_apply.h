#ifndef __LIBLLPP_TUPLE_APPLY_H__
#define __LIBLLPP_TUPLE_APPLY_H__

#include <tuple>
#include <type_traits>

namespace ll {
    struct tuple_apply {
        // tuple expand at back of args.
        struct back {
            template<typename _F, typename _T, unsigned __n, unsigned __i>
            struct Apply {
                template<typename... _A>
                static inline auto apply(_F && f, _T && t, _A &&... a)
                    -> decltype(Apply<_F, _T, __n-1, __i+1>::apply (
                        std::forward<_F>(f), 
                        std::forward<_T>(t),
                        std::forward<_A>(a)...,
                        std::get<__i>(std::forward<_T>(t))
                        )) {

                    return Apply<_F, _T, __n-1, __i+1>::apply (
                        std::forward<_F>(f), 
                        std::forward<_T>(t), 
                        std::forward<_A>(a)...,
                        std::get<__i>(std::forward<_T>(t))
                        );
                }
            };

            template<typename _F, typename _T, unsigned __i>
            struct Apply<_F, _T, 0, __i> {
                template<typename... _A>
                static inline auto apply(_F && f, _T &&, _A &&... a)
                    -> decltype(std::forward<_F>(f)(std::forward<_A>(a)...)) {
                    return std::forward<_F>(f)(std::forward<_A>(a)...);
                }
            };

            template<typename _F, typename _T, typename ..._Args>
            static inline auto apply(_F && f, _T && t, _Args&&...args) -> decltype(
                Apply<_F, _T, std::tuple_size<typename std::decay<_T>::type>::value, 0>::apply(
                    std::forward<_F>(f), 
                    std::forward<_T>(t),
                    std::forward<_Args>(args)...)) {
                return Apply<_F, _T, std::tuple_size<typename std::decay<_T>::type>::value, 0>::apply(
                    std::forward<_F>(f), 
                    std::forward<_T>(t),
                    std::forward<_Args>(args)...);
            }

        };

        // tuple expand at front of args.
        struct front {
            template<typename _F, typename _T, unsigned __n>
            struct Apply {
                template<typename... _A>
                static inline auto apply(_F && f, _T && t, _A &&... a)
                    -> decltype(Apply<_F, _T, __n - 1>::apply (
                        std::forward<_F>(f), 
                        std::forward<_T>(t),
                        std::get<__n - 1>(std::forward<_T>(t)),
                        std::forward<_A>(a)...)) {

                    return Apply<_F, _T, __n - 1>::apply (
                        std::forward<_F>(f), 
                        std::forward<_T>(t), 
                        std::get<__n - 1>(std::forward<_T>(t)),
                        std::forward<_A>(a)...);
                }
            };

            template <typename _F, typename _T>
            struct Apply<_F, _T, 0> {
                template<typename... _A>
                static inline auto apply(_F && f, _T &&, _A &&... a)
                    -> decltype(std::forward<_F>(f)(std::forward<_A>(a)...)) {
                    return std::forward<_F>(f)(std::forward<_A>(a)...);
                }
            };

            template<typename _F, typename _T, typename ..._Args>
            static inline auto apply(_F && f, _T && t, _Args&&...args) -> decltype(
                Apply<_F, _T, std::tuple_size<typename std::decay<_T>::type>::value>::apply(
                    std::forward<_F>(f), 
                    std::forward<_T>(t),
                    std::forward<_Args>(args)...)) {
                return Apply<_F, _T, std::tuple_size<typename std::decay<_T>::type>::value>::apply(
                    std::forward<_F>(f), 
                    std::forward<_T>(t),
                    std::forward<_Args>(args)...);
            }
        };

        template<bool __back = false, typename _F, typename _T, typename ..._Args>
        static inline auto apply(_F && f, _T && t, _Args&&...args) -> decltype(
            std::conditional<__back, back, front>::type::apply(
                std::forward<_F>(f), 
                std::forward<_T>(t), 
                std::forward<_Args>(args)...)) {
            
            return std::conditional<__back, back, front>::type::apply(
                std::forward<_F>(f), 
                std::forward<_T>(t), 
                std::forward<_Args>(args)...);
        }
    };
};


#endif

