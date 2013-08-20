#include <cassert>
#include "obstack.h"
#include "printf.h"

namespace ll {

obstack::obstack(page_allocator *allocator)
{
    if (!allocator) {
        allocator = page_allocator::instance();
    }

    page *chunk; 
    int size = page_allocator::page_min_size;

    chunk = allocator->alloc(size);
    chunk->next = nullptr;

    _allocator = allocator;
    _chunk = chunk;
    _next_free = _object_base = chunk->firstp;
    _chunk_limit = chunk->endp;
    _maybe_empty_object = false;
}

obstack::~obstack() {
    free(nullptr);
}

void obstack::newchunk(size_t length)
{
    register page *old_chunk = _chunk;
    register page *new_chunk;
    register size_t new_size;
    register size_t obj_size = _next_free - _object_base;
    char *object_base;

    new_size = obj_size + length;
    new_chunk = _allocator->alloc(new_size);

    _chunk = new_chunk;
    new_chunk->next = old_chunk;
    _chunk_limit = new_chunk->endp;

    /* Compute an aligned object_base in the new chunk */
    object_base = new_chunk->firstp;
    memcpy(object_base, _object_base, obj_size);

    if (!_maybe_empty_object && (_object_base == old_chunk->firstp)) {
        new_chunk->next = old_chunk->next;
        _allocator->free(old_chunk);
    }

    _object_base = object_base;
    _next_free = _object_base + obj_size;
    _maybe_empty_object = false;

}

void obstack::free(page *chunk, char *obj)
{
    page *tmp;
    do {
        tmp = chunk->next;
        _allocator->free(chunk);
        chunk = tmp;
        /* If we switch chunks, we can't tell whether the new current
           chunk contains an empty object, so assume that it may.  */
        _maybe_empty_object = 1;
    } while (chunk && (chunk->firstp >= obj || chunk->endp < obj));

    if (chunk) {
        _object_base = _next_free = obj;
        _chunk_limit = chunk->endp;
        _chunk = chunk;
    } else if (obj != 0) {
        /* obj is not in any of the chunks! */
        assert(0);
    }
}

struct obstack_vbuff : public printf_formatter::buff {
    obstack *_owner;

    int flush() {
        _owner->_next_free = curpos;
        if (_owner->_next_free >= _owner->_chunk_limit) {
            _owner->make_rome(128);
            curpos = _owner->_next_free;
            endpos = _owner->_chunk_limit;
        }
        return 0;
    }
};

int obstack::vsprintf(const char *fmt, va_list ap)
{
    obstack_vbuff buff;

    buff._owner = this;
    buff.curpos = _next_free;
    buff.endpos = _chunk_limit;

    int n;
    if ((n = printf_formatter::format(&buff, fmt, ap)) >= 0) {
        _next_free = buff.curpos;
    }
    return n;
}

int obstack::sprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int n = vsprintf(fmt, ap);
    va_end(ap);
    return n;
}

};

