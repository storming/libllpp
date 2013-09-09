#ifndef __LIBLLPP_MODULE_H__
#define __LIBLLPP_MODULE_H__

#include "slotsig.h"
#include "member.h"
#include <iostream>

namespace ll {

namespace internal {
namespace module {
extern bool __modules_reverse;
inline void foo() {
    __modules_reverse = __modules_reverse;
}
#ifdef __LIBLLPP__
struct modules_runner {
    static unsigned _counter;
    modules_runner(bool end);
    ~modules_runner();
};
#endif
};
};

template <typename ..._Args>
class modules {
public:
    typedef modules<_Args...> modules_t;
    typedef signal<int(_Args...)> sig_init_t;
    typedef signal<int(_Args...)> sig_exit_t;

    signal<int(_Args...)> sig_init;
    signal<int(_Args...)> sig_exit;

    modules() : sig_init(), sig_exit() {}

    int init(_Args&&...args) {
        if (ll::internal::module::__modules_reverse) {
            return sig_init.remit(std::forward<_Args>(args)...);
        }
        else {
            return sig_init.emit(std::forward<_Args>(args)...);
        }
    }
    int exit(_Args&&...args) {
        if (ll::internal::module::__modules_reverse) {
            return sig_exit.emit(std::forward<_Args>(args)...);
        }
        else {
            return sig_exit.remit(std::forward<_Args>(args)...);
        }
    }

    MEMBER_CHECKER_DECL(init_checker, module_init);
    MEMBER_CHECKER_DECL(exit_checker, module_exit);

    template <typename _T, typename = std::true_type>
    struct assember_init {
        template <typename ..._Params>
        static void assembe_init(modules_t &mm, _T &obj, _Params&&...params) {
            static typename sig_init_t::template slot<_T, _Params...> s(obj, &_T::module_init, std::forward<_Params>(params)...);
            mm.sig_init.connect(s);
        }
    };

    template <typename _T>
    struct assember_init<_T, std::integral_constant<bool, !init_checker<_T>::has_function::value>> {
        template <typename ..._Params>
        static void assembe_init(modules_t &mm, _T &obj, _Params&&...params) {
        }
    };

    template <typename _T, typename = std::true_type>
    struct assember_exit {
        template <typename ..._Params>
        static void assembe_exit(modules_t &mm, _T &obj, _Params&&...params) {
            static typename sig_exit_t::template slot<_T, _Params...> s(obj, &_T::module_init, std::forward<_Params>(params)...);
            mm.sig_exit.connect(s);
        }
    };

    template <typename _T>
    struct assember_exit<_T, std::integral_constant<bool, !exit_checker<_T>::has_function::value>> {
        template <typename ..._Params>
        static void assembe_exit(modules_t &mm, _T &obj, _Params&&...params) {
        }
    };

    template <typename _T>
    class assember : public assember_init<_T>, public assember_exit<_T> {
    public:
        template <typename ..._Params>
        assember(modules_t &mm, _Params&&...params) {
            static _T obj;
            assember_init<_T>::assembe_init(mm, obj, std::forward<_Params>(params)...);
            assember_exit<_T>::assembe_exit(mm, obj, std::forward<_Params>(params)...);
        }
    };
};

#ifdef __LIBLLPP__

namespace internal {
namespace module {
class modules : public ll::modules<> {
public:
    static modules &instance() {
        static modules m;
        return m;
    }
};
};
};

#define ll_module(mclass, ...) static ll::internal::module::modules::assember<mclass> \
    __libllpp_module_##mclass(ll::internal::module::modules::instance(), ##__VA_ARGS__)
#endif

};

#endif

