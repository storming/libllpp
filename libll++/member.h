#ifndef __LIBLLPP_MEMBER_H__
#define __LIBLLPP_MEMBER_H__

#include <type_traits>

namespace ll {

template <typename _T>
struct member_of;

template <typename _T, typename _U>
struct member_of<_T _U::*> {
    typedef _T type;
    typedef _U class_type;
};

/* offsetof_member */
template <typename _T, typename _C>
constexpr unsigned offsetof_member(_T _C::*member) {
    static_assert(std::is_member_object_pointer<decltype(member)>::value, 
                  "offsetof_member only use for member object pointer.");
    return reinterpret_cast<unsigned>(&(((_C*)0)->*member));
};

/* container_of */
template <typename _T, typename _C>
inline _C *containerof_member(_T *ptr, _T _C::*member) {
    return reinterpret_cast<_C*>(reinterpret_cast<char*>(ptr) - offsetof_member(member));
}

/* member check */
#define MEMBER_CHECKER_DECL(class_name, member)                                                                                                \
template <class T>                                                                                                                             \
struct class_name {                                                                                                                            \
    template <typename... Args>                                                                                                                \
    struct ambiguate : public Args... {};                                                                                                      \
                                                                                                                                               \
    template<typename A, typename = void>                                                                                                      \
    struct got_type : std::false_type {};                                                                                                      \
                                                                                                                                               \
    template<typename A>                                                                                                                       \
    struct got_type<A> : std::true_type {                                                                                                      \
        typedef A type;                                                                                                                        \
    };                                                                                                                                         \
                                                                                                                                               \
    template<typename A, typename = std::true_type>                                                                                            \
    struct Alias;                                                                                                                              \
                                                                                                                                               \
    template<typename A>                                                                                                                       \
    struct Alias<A, std::integral_constant<bool, got_type<decltype(&A::member)>::value>> {                                                     \
        static const decltype(&A::member) value;                                                                                               \
    };                                                                                                                                         \
                                                                                                                                               \
    struct AmbiguitySeed { char member; };                                                                                                     \
                                                                                                                                               \
    struct _has {                                                                                                                              \
        template<typename C> static char ((&f(decltype(&C::value))))[1];                                                                       \
        template<typename C> static char ((&f(...)))[2];                                                                                       \
                                                                                                                                               \
        typedef Alias<ambiguate<T, AmbiguitySeed>> alias_t;                                                                                    \
        typedef Alias<AmbiguitySeed> seed_t;                                                                                                   \
                                                                                                                                               \
        /*Make sure the member name is consistently spelled the same.*/                                                                        \
        static_assert(                                                                                                                         \
            (sizeof(f<seed_t>(0)) == 1)                                                                                                        \
            , "Member name specified in AmbiguitySeed is different from member name specified in Alias, "                                      \
              "or wrong Alias/AmbiguitySeed has been specified.");                                                                             \
                                                                                                                                               \
        static constexpr bool value = sizeof(f<alias_t>(0)) == 2;                                                                              \
    };                                                                                                                                         \
    using has = std::integral_constant<bool, _has::value>;                                                                                     \
                                                                                                                                               \
    struct _has_variable {                                                                                                                     \
        template<typename A, typename = std::true_type>                                                                                        \
        struct checker: std::false_type {};                                                                                                    \
                                                                                                                                               \
        template<typename A>                                                                                                                   \
        struct checker<A, std::integral_constant<bool, !std::is_member_function_pointer<decltype(&A::member)>::value>>: std::true_type {};     \
        static constexpr bool value = checker<T>::value;                                                                                       \
    };                                                                                                                                         \
    using has_variable = std::integral_constant<bool, _has_variable::value>;                                                                   \
                                                                                                                                               \
    struct _has_class {                                                                                                                        \
        template<typename A, typename = std::true_type>                                                                                        \
        struct checker: std::false_type {};                                                                                                    \
                                                                                                                                               \
        template<typename A>                                                                                                                   \
        struct checker<A, std::integral_constant<bool, std::is_class<typename got_type<typename A::member>::type>::value>>: std::true_type {}; \
                                                                                                                                               \
        static constexpr bool value = checker<T>::value;                                                                                       \
    };                                                                                                                                         \
    using has_class = std::integral_constant<bool, _has_class::value>;                                                                         \
                                                                                                                                               \
    struct _has_union {                                                                                                                        \
        template<typename A, typename = std::true_type>                                                                                        \
        struct checker: std::false_type {};                                                                                                    \
                                                                                                                                               \
        template<typename A>                                                                                                                   \
        struct checker<A, std::integral_constant<bool, std::is_union<typename got_type<typename A::member>::type>::value>>: std::true_type {}; \
                                                                                                                                               \
        static constexpr bool value = checker<T>::value;                                                                                       \
    };                                                                                                                                         \
    using has_union = std::integral_constant<bool, _has_union::value>;                                                                         \
                                                                                                                                               \
    struct _has_enum {                                                                                                                         \
        template<typename A, typename = std::true_type>                                                                                        \
        struct checker: std::false_type {};                                                                                                    \
                                                                                                                                               \
        template<typename A>                                                                                                                   \
        struct checker<A, std::integral_constant<bool, std::is_enum<typename got_type<typename A::member>::type>::value>>: std::true_type {};  \
                                                                                                                                               \
        static constexpr bool value = checker<T>::value;                                                                                       \
    };                                                                                                                                         \
    using has_enum = std::integral_constant<bool, _has_enum::value>;                                                                           \
                                                                                                                                               \
    using has_function = std::integral_constant<bool, has::value &&                                                                            \
            !has_variable::value &&                                                                                                            \
            !has_class::value &&                                                                                                               \
            !has_union::value &&                                                                                                               \
            !has_enum::value>;                                                                                                                 \
                                                                                                                                               \
    template <typename A>                                                                                                                      \
    struct _has_signature;                                                                                                                     \
                                                                                                                                               \
    template <typename R, typename ...Args>                                                                                                    \
    struct _has_signature<R(Args...)> {                                                                                                        \
        template<typename A, A>                                                                                                                \
        struct sig_check : std::true_type {};                                                                                                  \
                                                                                                                                               \
        template<typename A, typename = std::true_type>                                                                                        \
        struct checker : std::false_type {};                                                                                                   \
                                                                                                                                               \
        template<typename A>                                                                                                                   \
        struct checker<A, std::integral_constant<bool, sig_check<R (A::*)(Args...), &A::member>::value>> : std::true_type {};                  \
                                                                                                                                               \
        static constexpr bool value = checker<T>::value;                                                                                       \
    };                                                                                                                                         \
                                                                                                                                               \
    template <typename A>                                                                                                                      \
    struct has_signature;                                                                                                                      \
                                                                                                                                               \
    template <typename R, typename ...Args>                                                                                                    \
    struct has_signature<R(Args...)> : std::integral_constant<bool, _has_signature<R(Args...)>::value> {};                                     \
}

};

#endif

