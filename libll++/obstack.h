#ifndef __LIBLLPP_OBSTACK_H__
#define __LIBLLPP_OBSTACK_H__

#include <cstdarg>
#include <utility>
#include "page.h"

namespace ll {

class obstack {
    friend class obstack_vbuff;
private:
    page_allocator *_allocator;
    page *_chunk;
    char *_object_base;
    char *_next_free;
    char *_chunk_limit;
    bool _maybe_empty_object;

    void newchunk(size_t length);
    void free(page *chunk, char *obj);
public:
    obstack(page_allocator *allocator = nullptr);
    ~obstack();
    
    int vsprintf(const char *fmt, va_list ap);
    int sprintf(const char *fmt, ...);

    void *base() {
         return (void*)_object_base;
    }

    void *next_free() {
        return _next_free;
    }

    size_t object_size() {
        return _next_free - _object_base;
    }

    size_t rome_size() {
        return _chunk_limit - _next_free;
    }

    void make_rome(size_t size) {
        if (ll_unlikely((size_t)(_chunk_limit - _next_free) < size)) {
            newchunk(size);
        }
    }

    void grow_fast(const void *p, size_t size) {
        memcpy(_next_free, p, size);
        _next_free += size;
    }

    void grow(const void *p, size_t size) {
        if (ll_unlikely(_next_free + size > _chunk_limit)) {
            newchunk(size);
        }
        memcpy(_next_free, p, size);
        _next_free += size;
    }

    void grow0(const void *p, size_t size) {
        if (ll_unlikely(_next_free + size + 1 > _chunk_limit)) {
            newchunk(size + 1);
        }
        memcpy(_next_free, p, size);
        _next_free += size;
        *_next_free++ = 0;
    }

    template <typename _T>
    void grow(_T obj) {
        static_assert(std::is_arithmetic<_T>::value || std::is_pointer<_T>::value, 
                      "grow must is arithmetic or pointer.");
        if (ll_unlikely(_next_free + sizeof(_T) > _chunk_limit)) {
            newchunk(sizeof(_T));
        }
        *((_T*)_next_free) = obj;
        _next_free += sizeof(_T);
    }

    template <typename _T>
    obstack& operator <<(_T obj) {
        grow(obj);
        return *this;
    }

    void blank_fast(size_t size) {
        _next_free += size;
    }

    void blank(size_t size) {
        if (ll_unlikely((size_t)(_chunk_limit - _next_free) < size)) {
            newchunk(size);
        }
        _next_free += size;
    }

    void *alloc(size_t size) {
        if (ll_unlikely((size_t)(_chunk_limit - _next_free) < size)) {
            newchunk(size);
        }
        _next_free += size;
        return finish();
    }

    void *calloc(size_t size) {
        void *p = alloc(size);
        memset(p, 0, size);
        return p;
    }

    void *copy(const void *p, size_t size) {
        grow(p, size);
        return finish();
    }

    void *copy0(const void *p, size_t size) {
        grow0(p, size);
        return finish();
    }

    void *finish() {
        char *p = _object_base;
        if (ll_unlikely(_next_free == p)) {
            _maybe_empty_object = true;
        }

        _next_free = (char*)ll_align_default((intptr_t)_next_free);
        if (ll_unlikely(_next_free > _chunk_limit)) {
            _next_free = _chunk_limit;
        }
        _object_base = _next_free;
        return (void*)p;
    }

    void free(void *obj) {
        char *p = (char*)obj;
        if (p > _chunk->firstp && p < _chunk_limit) {
            _next_free = _object_base = p;
        }
        else {
            free(_chunk, p);
        }
    }

    char *strdup(const char *s) {
        return (char*)copy0(s, strlen(s));
    }
};

};

#endif
