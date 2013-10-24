#ifndef __LIBLLPP_GUARD_H__
#define __LIBLLPP_GUARD_H__

#include "tuple_apply.h"
#include "functor.h"

namespace ll {

template <typename _Functor>
class guard {
public:
    typedef void signature(void);

private:
    _Functor _fn;
public:
    guard(const guard&) = delete;
    guard(guard &&x) noexcept : _fn(std::move(x._fn)) {}

    template <typename _T, typename ..._Params>
    guard(_T &&fn, _Params&&...params) noexcept
        : _fn(std::forward<_T>(fn), std::forward<_Params>(params)...) {}

    ~guard() noexcept {
        if (_fn != nullptr) {
            _fn();
        }
    }
    void operator()() noexcept {
        _fn();
    }
    
    void dismiss() noexcept {
        _fn = nullptr;
    }
};

template <typename _T, typename ..._Params>
inline auto make_guard(_T &&fn, _Params&&...params) noexcept ->
    guard<decltype(make_functor<void(void)>(
        std::forward<_T>(fn), 
        std::forward<_Params>(params)...))>
{
    typedef decltype(make_functor<void(void)>(
        std::forward<_T>(fn), 
        std::forward<_Params>(params)...)) functor_type;

    return guard<functor_type>(std::forward<_T>(fn), std::forward<_Params>(params)...);
}

}


#endif
