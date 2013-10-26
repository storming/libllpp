#ifndef __LIBLLPP_CONFIG_FILE_H__
#define __LIBLLPP_CONFIG_FILE_H__

#include "list.h"
#include "obstack.h"

namespace ll {

struct config_pos {
    const char *file;
    unsigned line;
    unsigned column;
};

struct config_loc {
    config_pos start;
    config_pos end;
};

class config_text {
    friend class config_loader;
    const char *_text;
public:
    config_loc loc;
    operator const char*() {
        return _text;
    }
    const char* text() {
        return _text;
    }
};

class config_item_base {
    friend class config_loader;
    clist_entry entry;
public:
    config_text *name;
    config_text **values;
    unsigned value_count;
    config_item_base() : name(), values(), value_count() {}

    operator unsigned() {
        return value_count;
    }

    config_text &operator[](unsigned index) {
        assert(values && index < value_count);
        return *(values[index]);
    }

    operator const char*() {
        return *name;
    }
};

template <typename _T>
class config_items_impl : protected ll_list(config_item_base, entry, _T) {
    friend class config_loader;
private:
    typedef typename ll_list(config_item_base, entry, _T) base_type;
public:
    using typename base_type::begin;
    using typename base_type::end;
    using typename base_type::empty;
    using typename base_type::init;
};

class config_item : public config_item_base {
    friend class config_loader;
private:
    config_item *_parent;
public:
    config_items_impl<config_item> children;

    config_item(config_item *parent) : _parent(parent), children() {}
    config_item *parent() {
        return _parent;
    }
};

using config_items = config_items_impl<config_item>;

class config_doc {
private:
    obstack *_pool;
public:
    config_items items;
    config_doc();
    ~config_doc();
    void clear();
    int load(const char *filename, char **msg = nullptr);
};

}

#endif
