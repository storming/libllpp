#ifndef __LIBLLPP_ALLOCATOR_BIND_H__
#define __LIBLLPP_ALLOCATOR_BIND_H__

namespace ll {

template <typename _Alloc>
class allocator_bind {
private:
    typedef _Alloc allocator_type;
    allocator_type *_a;

public:
    explicit allocator_bind() : _a() {}
    explicit allocator_bind(allocator_type *ma) : _a(ma) {}
    explicit allocator_bind(const allocator_bind &o) : _a(o._a) {}

    void *alloc(size_t size) noexcept {
        return _a->alloc(size);
    }

    void *alloc(size_t size) const noexcept {
        return _a->alloc(size);
    }

    void free(void *p, size_t size) noexcept {
        _a->free(p, size);
    }

    void free(void *p, size_t size) const noexcept {
        _a->free(p, size);
    }

};

}

#endif
