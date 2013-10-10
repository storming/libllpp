#ifndef __LIBLLPP_MAP_H__
#define __LIBLLPP_MAP_H__

#include "rbtree.h"
#include "allocator.h"
#include "compare.h"

namespace ll {

typedef rbtree::node map_entry;

namespace map_helper {
    template <
        typename _Key, 
        typename _T, 
        typename _Entry, 
        _Entry _Base::*__field, 
        bool __left_acc, 
        bool __right_acc, 
        typename _Allocator,
        typename _GetKey,
        typename _Compare>
    struct impl {
        typedef _Key                            key_type;
        typedef _T                              value_type;
        typedef _Entry                          entry_type;
        typedef _Allocator                      allocator_type;
        typedef _GetKey                         getkey_type;
        typedef _Compare                        compare_type;
        class map;

        template <typename _AEntry = entry_type, bool = __left_acc>
        struct left_acc_policy {
            map_entry *_left;
            left_acc_policy() : _left() {}

            void update(map_entry *node) {
                _left = node;
            }

            map_entry *get() {
                return _left;
            }

            void init() {
                _left = nullptr;
            }
        };

        template <typename _AEntry>
        struct left_acc_policy<_AEntry, false> {
            void update(map_entry *entry);
            map_entry *get();
            void init();
        };

        template <typename _AEntry = entry_type, bool = __right_acc>
        struct right_acc_policy {
            map_entry *_right;

            right_acc_policy() : _right() {}
            void update(map_entry *node) {
                _right = node;
            }

            map_entry *get() {
                return _right;
            }

            void init() {
                _right = nullptr;
            }
        };

        template <typename _AEntry>
        struct right_acc_policy<_AEntry, false> {
            void update(map_entry *entry);
            map_entry *get();
            void init();
        };

        class map {
        public:
            class iterator {
                friend class map;
            private:
                map_entry *_node;
                iterator(map_entry *node) : _node(node) {}
            public:
                iterator() : _node() {}
                value_type& operator*() {
                    return *(static_cast<value_type*>(containerof_member(static_cast<entry_type*>(_node), __field)));
                }

                value_type* operator->() {
                    return static_cast<value_type*>(containerof_member(static_cast<entry_type*>(_node), __field));
                }

                value_type* pointer() {
                    return static_cast<value_type*>(containerof_member(static_cast<entry_type*>(_node), __field));
                }

                const iterator& operator++() {
                    _node = _node->next();
                    return *this;
                }

                iterator operator++(int) {
                    iterator tmp(_node);
                    _node = _node->next();
                    return tmp;
                }

                iterator& operator--() {
                    _node = _node->prev();
                    return *this;
                }

                iterator operator--(int) {
                    iterator tmp(_node);
                    _node = _node->prev();
                    return tmp;
                }

                bool operator==(const iterator &other) const {
                    return _node == other._node;
                }

                bool operator!=(const iterator &other) const {
                    return _node != other._node;
                }
            };

        private:
            struct impl : public rbtree, public left_acc_policy<>, public right_acc_policy<>, public allocator_type {
                impl() : rbtree(), left_acc_policy<>(), right_acc_policy<>, allocator_type() {}
                impl(const allocator_type &a) : rbtree(), left_acc_policy<>(), right_acc_policy<>, allocator_type(a) {}

                using rbtree::_root;
                using rbtree::front;
                using rbtree::back;

                void init() {
                    rbtree::init();

                    if (__left_acc) {
                        left_acc_policy<>::init();
                    }

                    if (__right_acc) {
                        right_acc_policy<>::init();
                    }
                }

                map_entry *get_left() {
                    if (__left_acc) {
                        return left_acc_policy<>::get();
                    }
                    else {
                        return front();
                    }
                }

                map_entry *get_right() {
                    if (__right_acc) {
                        return right_acc_policy<>::get();
                    }
                    else {
                        return back();
                    }
                }

                void set_left(map_entry *node) {
                    left_acc_policy<>::update(node);
                }

                void set_right(map_entry *node) {
                    return right_acc_policy<>::update(node);
                }
            };

            impl _impl;
        public:
            map() : _impl() {}
            map(const allocator_type &a) : _impl(a) {}

            using rbtree::empty;
            
            void init() {
                _impl.init();
            }

            value_type *front() {
                return containerof_member(static_cast<entry_type*>(_impl.get_left()), __field);
            }

            value_type *back() {
                return containerof_member(static_cast<entry_type*>(_impl.get_right()), __field);
            }

            value_type* get(key_type key) {
                map_entry *node = _impl._root;
                value_type *elm;
                int n;

                while (node) {
                    elm = containerof_member(static_cast<entry_type*>(node), __field);
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
            value_type *probe(key_type key, int *flag, Args &&... args) {
                map_entry **link = &_impl._root;
                map_entry *parent = nullptr;
                value_type* elm;
                int left __attribute__((unused)) = 1;
                int right __attribute__((unused)) = 1;
                int n;

                while (*link) {
                    parent = *link;
                    elm = containerof_member(static_cast<entry_type*>(parent), __field);
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

                rbtree::link(node, parent, link);
                rbtree::insert(node);

                if (flag) {
                    *flag = 1;
                }
                return elm;
            }

            template <bool __insert_after = true>
            value_type* insert(value_type* new_elm) {
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
    
                        elm = containerof_member(static_cast<entry_type*>(parent), __field);
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
    
                        elm = containerof_member(static_cast<entry_type*>(parent), __field);
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
    
                rbtree::link(node, parent, link);
                rbtree::insert(node);
    
                return elm;
            }
    
            value_type* replace(value_type *maped, value_type *new_elm) {
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
    
            value_type* replace(value_type *new_elm) {
                key_type key = getkey_type::get_key(new_elm);
                map_entry **link = &_impl._root;
                map_entry *parent = nullptr;
                int left __attribute__((unused)) = 1;
                int right __attribute__((unused)) = 1;
                int n;
                value_type* elm;
    
                while (*link) {
                    parent = *link;
    
                    elm = containerof_member(static_cast<entry_type*>(parent), __field);
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
    
                rbtree::link(node, parent, link);
                rbtree::insert(node);
    
                return nullptr;
            }
    
            value_type* remove(value_type *elm) {
                map_entry *node = &(elm->*__field);
    
                if (__left_acc && _impl.get_left() == node) {
                    _impl.set_left(node->next());
                }
                if (__right_acc && right_acc_policy<>::get() == node) {
                    _impl.set_right(node->prev());
                }
                _impl.remove(node);
                return elm;
            }
    
            value_type* remove(key_type key) {
                value_type* elm = get(key);
                if (elm) {
                    remove(elm);
                }
                return elm;
            }
    
            iterator begin() {
                return iterator(_impl.get_left());
            }
    
            const iterator begin() const {
                return iterator(_impl.get_left());
            }
    
            iterator end() {
                return iterator(nullptr);
            }
    
            const iterator end() const {
                return iterator(nullptr);
            }
    
            iterator rbegin() {
                return iterator(_impl.get_right());
            }
    
            const iterator rbegin() const {
                return iterator(_impl.get_right());
            }
    
            iterator rend() {
                return iterator(nullptr);
            }
    
            const iterator rend() const {
                return iterator(nullptr);
            }
        };
    };
}

template <
    typename _Key, 
    typename _T, 
    typename _Base, 
    typename _Entry, 
    _Entry _Base::*__field, 
    typename _Allocator = allocator_of<_T>::type,
    bool __left_acc = true, 
    bool __right_acc = false, 
    typename _Compare = comparer<_Key>,
    typename _GetKey = _T>
class map : public map_helper::impl<_Key, _T, _Base, _Entry, __field, __left_acc, __right_acc, _Allocator, _GetKey, _Compare>::map {
public:
    using map_helper::impl<_Key, _T, _Entry, __field, __left_acc, __right_acc, _Allocator, _GetKey, _Compare>::map;
};

#define ll_map(k, T, entry, ...) ll::map<                    \
    k, T,                                                    \
    typename ll::member_of<decltype(&T::entry)>::class_type, \
    typename ll::member_of<decltype(&T::entry)>::type,       \
    &T::entry,                                               \
    ##__VA_ARGS__>

}

#endif
