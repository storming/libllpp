#ifndef __LIBLLPP_PAGE_H__
#define __LIBLLPP_PAGE_H__

#include <type_traits>
#include <cstdint>
#include <cassert>
#include <string.h>

#include "list.h"
#include "etc.h"
#include "bitorder.h"
#include "friend.h"

namespace ll {

struct page {
    friend class page_allocator;
    union {
        list_entry   entry;
        slist_entry  sentry;
        stlist_entry stentry;
        clist_entry  centry;
    };
    char *firstp;
    char *endp;
private:
    unsigned order_size;
    void *base;
};

class page_allocator {
    LL_FRIEND_MODULES();
public:
    static const unsigned sys_page_size;
    static constexpr unsigned max_order            = 8;
    static constexpr unsigned min_order            = 1;
    static constexpr unsigned page_boundary_index  = 12;
    static constexpr unsigned page_min_size        = 1 << (page_boundary_index + min_order);
    static constexpr unsigned page_max_size        = 1 << (page_boundary_index + max_order);

    static constexpr unsigned iseg_initsize        = 1024 * 64;
    static constexpr unsigned iseg_expand_size     = 1024 * 64;

    static constexpr unsigned cseg_initsize        = page_max_size * 16;
    static constexpr unsigned cseg_expand_size     = page_max_size * 16;
private:
    struct segment {
        list_entry _entry;
        char *_base;
        char *_firstp;
        char *_endp;
    };
    typedef ll_list(segment, _entry) segmentlist_t;

    struct page_node {
        list_entry _entry;
        unsigned short _index;
        unsigned short _order;
    };

    struct area {
        static constexpr unsigned length = 1 << (max_order - min_order);

        page_node _pages[length];
        union {
            char *_base;
            slist_entry _entry;
        };
    };

    typedef ll_list(page_node, _entry) freelist_t;

    freelist_t _freetab[max_order];
    segment *_iseg;
    segment *_cseg;
    ll_list(page, sentry) _pages;
    ll_list(area, _entry) _areas;
    segmentlist_t _segments;

    segment *segment_create(segment *seg, unsigned size);
    void segment_destroy(segment *seg);
    bool segment_expand(segment *seg, unsigned size);

    void *ialloc(unsigned size);

    page *page_alloc();
    void page_free(page *pg);
    area *area_alloc(void *base);
    void area_free(area *a);
    void *chunk_alloc();
    void chunk_free(void *chunk);

    static page_allocator *_global;
    int module_init();
public:
    page_allocator();
    ~page_allocator();
    page *alloc(size_t size);
    void free(page *p);

    static page_allocator *global() {
        return _global;
    }
};

};


#endif

