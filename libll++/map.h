#ifndef __LIBLLPP_MAP_H__
#define __LIBLLPP_MAP_H__

#include "rbtree.h"
#include "allocator.h"
#include "compare.h"
#include "log.h"

namespace ll {

typedef rbtree::node map_entry;

#define __LL_MAP_OBJECT__(node) static_cast<value_type*>(containerof_member(static_cast<entry_type*>(node), __field))

template <
    typename _Key, 
    typename _T, 
    typename _Base, 
    typename _Entry, 
    _Entry _Base::*__field, 
    typename _Allocator = dumb_allocator,
    bool __left_acc = true, 
    bool __right_acc = false, 
    typename _Compare = comparer<_Key>,
    typename _GetKey = _T>
class map {
public:
    typedef _Key                            key_type;
    typedef _T                              value_type;
    typedef _Entry                          entry_type;
    typedef _Allocator                      allocator_type;
    typedef _GetKey                         getkey_type;
    typedef _Compare                        compare_type;

    class iterator {
        friend class map;
    private:
        map_entry *_node;
        iterator(map_entry *node) noexcept : _node(node) {}
    public:
        iterator() noexcept : _node() {}

        iterator& operator=(const iterator& x) noexcept {
            _node = x._node;
            return *this;
        }

        value_type& operator*() noexcept {
            return *(__LL_MAP_OBJECT__(_node));
        }

        value_type* operator->() noexcept {
            return __LL_MAP_OBJECT__(_node);
        }

        value_type* pointer() noexcept {
            return __LL_MAP_OBJECT__(_node);
        }

        const iterator& operator++() noexcept {
            _node = _node->next();
            return *this;
        }

        iterator operator++(int) noexcept {
            iterator tmp(_node);
            _node = _node->next();
            return tmp;
        }

        iterator& operator--() noexcept {
            _node = _node->prev();
            return *this;
        }

        iterator operator--(int) noexcept {
            iterator tmp(_node);
            _node = _node->prev();
            return tmp;
        }

        bool operator==(const iterator &other) const noexcept {
            return _node == other._node;
        }

        bool operator!=(const iterator &other) const noexcept {
            return _node != other._node;
        }
    };

    class reverse_iterator {
        friend class map;
    private:
        map_entry *_node;
        reverse_iterator(map_entry *node) noexcept : _node(node) {}
    public:
        reverse_iterator() noexcept : _node() {}
        
        reverse_iterator& operator=(const reverse_iterator& x) noexcept {
            _node = x._node;
            return *this;
        }

        value_type& operator*() noexcept {
            return *(__LL_MAP_OBJECT__(_node));
        }

        value_type* operator->() noexcept {
            return __LL_MAP_OBJECT__(_node);
        }

        value_type* pointer() noexcept {
            return __LL_MAP_OBJECT__(_node);
        }

        const reverse_iterator& operator--() noexcept {
            _node = _node->next();
            return *this;
        }

        reverse_iterator operator--(int) noexcept {
            reverse_iterator tmp(_node);
            _node = _node->next();
            return tmp;
        }

        reverse_iterator& operator++() noexcept {
            _node = _node->prev();
            return *this;
        }

        reverse_iterator operator++(int) noexcept {
            reverse_iterator tmp(_node);
            _node = _node->prev();
            return tmp;
        }

        bool operator==(const reverse_iterator &other) const noexcept {
            return _node == other._node;
        }

        bool operator!=(const reverse_iterator &other) const noexcept {
            return _node != other._node;
        }
    };
private:
    template <typename = entry_type, bool = __left_acc>
    struct left_acc_policy {
        map_entry *_left;
        left_acc_policy() noexcept : _left() {}

        void update(map_entry *node) noexcept {
            _left = node;
        }

        map_entry *get() noexcept {
            return _left;
        }

        void init() noexcept {
            _left = nullptr;
        }
    };

    template <typename _AEntry>
    struct left_acc_policy<_AEntry, false> {
        void update(map_entry *entry);
        map_entry *get();
        void init();
    };

    template <typename = entry_type, bool = __right_acc>
    struct right_acc_policy {
        map_entry *_right;

        right_acc_policy() noexcept : _right() {}
        void update(map_entry *node) noexcept {
            _right = node;
        }

        map_entry *get() noexcept {
            return _right;
        }

        void init() noexcept {
            _right = nullptr;
        }
    };

    template <typename _AEntry>
    struct right_acc_policy<_AEntry, false> {
        void update(map_entry *entry);
        map_entry *get();
        void init();
    };

    struct impl : public rbtree, public left_acc_policy<>, public right_acc_policy<>, public allocator_type {
        impl() noexcept : 
            rbtree(), left_acc_policy<>(), right_acc_policy<>(), allocator_type() {}
        impl(allocator_type &a) noexcept : 
            rbtree(), left_acc_policy<>(), right_acc_policy<>(), allocator_type(a) {}
        impl(const allocator_type &a) noexcept : 
            rbtree(), left_acc_policy<>(), right_acc_policy<>(), allocator_type(a) {}

        using rbtree::_root;
        using rbtree::front;
        using rbtree::back;
        using rbtree::insert;
        using rbtree::link;

        void init() noexcept {
            rbtree::init();

            if (__left_acc) {
                left_acc_policy<>::init();
            }

            if (__right_acc) {
                right_acc_policy<>::init();
            }
        }

        map_entry *get_left() noexcept {
            if (__left_acc) {
                return left_acc_policy<>::get();
            }
            else {
                return front();
            }
        }

        map_entry *get_right() noexcept {
            if (__right_acc) {
                return right_acc_policy<>::get();
            }
            else {
                return back();
            }
        }

        void set_left(map_entry *node) noexcept {
            left_acc_policy<>::update(node);
        }

        void set_right(map_entry *node) noexcept {
            return right_acc_policy<>::update(node);
        }
    };

    impl _impl;
public:
    map() noexcept : _impl() {}
    map(allocator_type &a) noexcept : _impl(a) {}
    map(const allocator_type &a) noexcept : _impl(a) {}
   
    void init() noexcept {
        _impl.init();
    }

    value_type *front() noexcept {
        map_entry *node = _impl.get_left();
        return node ? __LL_MAP_OBJECT__(node) : nullptr;
    }

    value_type *back() noexcept {
        map_entry *node = _impl.get_right();
        return node ? __LL_MAP_OBJECT__(_impl.get_right()) : nullptr;
    }

    value_type* get(key_type key) noexcept {
        map_entry *node = _impl._root;
        value_type *elm;
        int n;

        while (node) {
            elm = __LL_MAP_OBJECT__(node);
            n = compare_type::compare(key, getkey_type::get_key(elm));

            if (n < 0) {
                node = node->_left;
            } else if (n > 0) {
                node = node->_right;
            }
            else {
                return elm;
            }
        }
        return nullptr;
    }

    template <typename ...Args>
    value_type *probe(key_type key, int *flag, Args &&... args) noexcept {
        map_entry **link = &_impl._root;
        map_entry *parent = nullptr;
        value_type* elm;
        int left __attribute__((unused)) = 1;
        int right __attribute__((unused)) = 1;
        int n;

        while (*link) {
            parent = *link;
            elm = __LL_MAP_OBJECT__(parent);
            n = compare_type::compare(key, getkey_type::get_key(elm));

            if (n < 0) {
                link = &(*link)->_left;
                if (__right_acc) {
                    right = 0;
                }
            } else if (n > 0) {
                link = &(*link)->_right;
                if (__left_acc) {
                    left = 0;
                }
            }
            else {
                if (flag) {
                    *flag = 0;
                }
                return elm;
            }
        }

        elm = _new<value_type>(_impl, key, std::forward<Args>(args)...);
        map_entry *node = &(elm->*__field);

        if (__left_acc && left) {
            _impl.set_left(node);
        }

        if (__right_acc && right) {
            _impl.set_right(node);
        }

        _impl.link(node, parent, link);
        _impl.insert(node);

        if (flag) {
            *flag = 1;
        }
        return elm;
    }

    template <bool __insert_after = true>
    value_type* insert(value_type* new_elm) noexcept {
        key_type key = getkey_type::get_key(new_elm);
        map_entry **link = &_impl._root;
        map_entry *parent = nullptr;
        int left __attribute__((unused)) = 1;
        int right __attribute__((unused)) = 1;
        int n;
        value_type* elm;

        if (__insert_after) {
            while (*link) {
                parent = *link;

                elm = __LL_MAP_OBJECT__(parent);
                n = compare_type::compare(key, getkey_type::get_key(elm));

                if (n < 0) {
                    link = &(*link)->_left;
                    if (__right_acc) {
                        right = 0;
                    }
                } else {
                    link = &(*link)->_right;
                    if (__left_acc) {
                        left = 0;
                    }
                }
            }
        }
        else {
            while (*link) {
                parent = *link;

                elm = __LL_MAP_OBJECT__(parent);
                n = compare_type::compare(key, getkey_type::get_key(elm));

                if (n <= 0) {
                    link = &(*link)->_left;
                    if (__right_acc) {
                        right = 0;
                    }
                } else {
                    link = &(*link)->_right;
                    if (__left_acc) {
                        left = 0;
                    }
                }
            }
        }

        elm = new_elm;
        map_entry *node = &(elm->*__field);

        if (__left_acc && left) {
            _impl.set_left(node);
        }

        if (__right_acc && right) {
            _impl.set_right(node);
        }

        _impl.link(node, parent, link);
        _impl.insert(node);

        return elm;
    }

    value_type* replace(value_type *maped, value_type *new_elm) noexcept {
        map_entry *maped_node = &(maped->*__field);
        map_entry *new_node = &(new_elm->*__field);
        if (__left_acc && _impl.get_left() == maped_node) {
            _impl.set_left(new_node);
        }
        if (__right_acc && _impl.get_right() == maped_node) {
            _impl.set_right(new_node);
        }

        _impl.replace(maped_node, new_node);
        return maped;
    }

    value_type* replace(value_type *new_elm) noexcept {
        key_type key = getkey_type::get_key(new_elm);
        map_entry **link = &_impl._root;
        map_entry *parent = nullptr;
        int left __attribute__((unused)) = 1;
        int right __attribute__((unused)) = 1;
        int n;
        value_type* elm;

        while (*link) {
            parent = *link;

            elm = __LL_MAP_OBJECT__(parent);
            n = compare_type::compare(key, getkey_type::get_key(elm));

            if (n < 0) {
                link = &(*link)->_left;
                if (__right_acc) {
                    right = 0;
                }
            } else if (n > 0) {
                link = &(*link)->_right;
                if (__left_acc) {
                    left = 0;
                }
            }
            else {
                return replace(elm, new_elm);
            }
        }

        map_entry *node = &(new_elm->*__field);

        if (__left_acc && left) {
            _impl.set_left(node);
        }

        if (__right_acc && right) {
            _impl.set_right(node);
        }

        _impl.link(node, parent, link);
        _impl.insert(node);

        return nullptr;
    }

    value_type* remove(value_type *elm) noexcept {
        map_entry *node = &(elm->*__field);

        if (__left_acc && _impl.get_left() == node) {
            _impl.set_left(node->next());
        }
        if (__right_acc && _impl.get_right() == node) {
            _impl.set_right(node->prev());
        }
        _impl.remove(node);
        return elm;
    }

    value_type* remove(key_type key) noexcept {
        value_type* elm = get(key);
        if (elm) {
            remove(elm);
        }
        return elm;
    }

    iterator begin() noexcept {
        return iterator(_impl.get_left());
    }

    const iterator begin() const noexcept {
        return iterator(_impl.get_left());
    }

    iterator end() noexcept {
        return iterator(nullptr);
    }

    const iterator end() const noexcept {
        return iterator(nullptr);
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(_impl.get_right());
    }

    const reverse_iterator rbegin() const noexcept {
        return reverse_iterator(_impl.get_right());
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(nullptr);
    }

    const reverse_iterator rend() const noexcept {
        return reverse_iterator(nullptr);
    }
};

#define ll_map(k, T, entry, ...) ll::map<                    \
    k, T,                                                    \
    typename ll::member_of<decltype(&T::entry)>::class_type, \
    typename ll::member_of<decltype(&T::entry)>::type,       \
    &T::entry,                                               \
    ##__VA_ARGS__>

}

#endif
