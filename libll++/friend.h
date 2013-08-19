#ifndef __LIBLLPP_FRIEND_H__
#define __LIBLLPP_FRIEND_H__

#define LL_FRIEND_TUPLE_APPLY()        \
    friend class ll:tuple_apply;       \
    friend class ll:tuple_apply::back; \
    friend class ll:tuple_apply::front

#define LL_FRIEND_MODULES()            \
    template <typename...> friend class modules;

#define LL_FRIEND_CLOSURE()            \
    LL_FRIEND_TUPLE_APPLY();           \
    template <typename F, typename ...Params> friend class ll::closure

#endif
