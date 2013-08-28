#ifndef __LIBLLPP_MAP_H__
#define __LIBLLPP_MAP_H__

#include "rbtree.h"
#include "memory.h"
#include "compare.h"

namespace ll {

typedef rbtree::node map_entry;

namespace map_helper {
    template <
        typename _Key, 
        typename _T, 
        typename _Base, 
        typename _Entry, 
        _Entry _Base::*__field, 
        bool __left_acc, 
        bool __right_acc, 
        typename _Factory,
        typename _GetKey,
        typename _Compare>
    struct impl {
        typedef _Key                            key_t;
        typedef _T                              type_t;
        typedef _Base                           base_t;
        typedef _Entry                          entry_t;
        typedef _Factory                        factory_t;
        typedef typename factory_t::allocator_t factory_allocator_t;
        typedef _GetKey                         getkey_t;
        typedef _Compare                        compare_t;
        class map;

        template <typename _AEntry = entry_t, bool = __left_acc>
        struct left_acc_policy {
            rbtree::node *_left;
            left_acc_policy() : _left() {}

            void update(rbtree::node *node) {
                _left = node;
            }

            rbtree::node *get() {
                return _left;
            }

            void truncate() {
                _left = nullptr;
            }
        };

        template <typename _AEntry>
        struct left_acc_policy<_AEntry, false> {
            void update(rbtree::node *entry);
            rbtree::node *get();
            void truncate();
        };

        template <typename _AEntry = entry_t, bool = __right_acc>
        struct right_acc_policy {
            rbtree::node *_right;

            right_acc_policy() : _right() {}
            void update(rbtree::node *node) {
                _right = node;
            }

            rbtree::node *get() {
                return _right;
            }

            void truncate() {
                _right = nullptr;
            }
        };

        template <typename _AEntry>
        struct right_acc_policy<_AEntry, false> {
            void update(rbtree::node *entry);
            rbtree::node *get();
            void truncate();
        };

        class map : public rbtree, protected left_acc_policy<>, protected right_acc_policy<> {
        public:
            class iterator {
                friend class map;
            private:
                rbtree::node *_node;
                iterator(rbtree::node *node) : _node(node) {}
            public:
                iterator() : _node() {}
                type_t& operator*() {
                    return *(static_cast<type_t*>(containerof_member(static_cast<entry_t*>(_node), __field)));
                }

                type_t* operator->() {
                    return static_cast<type_t*>(containerof_member(static_cast<entry_t*>(_node), __field));
                }

                type_t* pointer() {
                    return static_cast<type_t*>(containerof_member(static_cast<entry_t*>(_node), __field));
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
        public:
            map() : rbtree(), left_acc_policy<>(), right_acc_policy<>() {}

            using rbtree::empty;
            
            void truncate() {
                rbtree::truncate();

                if (__left_acc) {
                    left_acc_policy<>::truncate();
                }

                if (__right_acc) {
                    right_acc_policy<>::truncate();
                }
            }

            type_t *front() {
                rbtree::node *node;
                if (__left_acc) {
                    node = left_acc_policy<>::get();
                }
                else {
                    node = rbtree::front();
                }
                return static_cast<type_t*>(containerof_member(static_cast<entry_t*>(node), __field));
            }

            type_t *back() {
                rbtree::node *node;
                if (__right_acc) {
                    node = right_acc_policy<>::get();
                }
                else {
                    node = rbtree::back();
                }
                return static_cast<type_t*>(containerof_member(static_cast<entry_t*>(node), __field));
            }

            type_t* get(key_t key) {
                rbtree::node *node = _root;
                type_t *elm;
                int n;

                while (node) {
                    elm = static_cast<type_t*>(containerof_member(static_cast<entry_t*>(node), __field));
                    n = compare_t::compare(key, getkey_t::get_key(elm));

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
            type_t *probe(key_t key, int *flag, factory_allocator_t *allocator, Args &&... args) {
                rbtree::node **link = &_root;
                rbtree::node *parent = nullptr;
                type_t* elm;
                int left __attribute__((unused)) = 1;
                int right __attribute__((unused)) = 1;
                int n;

                while (*link) {
                    parent = *link;
                    elm = static_cast<type_t*>(containerof_member(static_cast<entry_t*>(parent), __field));
                    n = compare_t::compare(key, getkey_t::get_key(elm));

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

                elm = create<type_t, factory_t>(allocator, key, std::forward<Args>(args)...);
                rbtree::node *node = &(elm->*__field);

                if (__left_acc && left) {
                    left_acc_policy<>::update(node);
                }

                if (__right_acc && right) {
                    right_acc_policy<>::update(node);
                }

                rbtree::link(node, parent, link);
                rbtree::insert(node);

                if (flag) {
                    *flag = 1;
                }
                return elm;
            }

            template <bool __insert_after = true>
            type_t* insert(type_t* new_elm) {
                key_t key = getkey_t::get_key(new_elm);
                rbtree::node **link = &_root;
                rbtree::node *parent = nullptr;
                int left __attribute__((unused)) = 1;
                int right __attribute__((unused)) = 1;
                int n;
                type_t* elm;
    
                if (__insert_after) {
                    while (*link) {
                        parent = *link;
    
                        elm = static_cast<type_t*>(containerof_member(static_cast<entry_t*>(parent), __field));
                        n = compare_t::compare(key, getkey_t::get_key(elm));
    
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
    
                        elm = static_cast<type_t*>(containerof_member(static_cast<entry_t*>(parent), __field));
                        n = compare_t::compare(key, getkey_t::get_key(elm));
    
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
                rbtree::node *node = &(elm->*__field);
    
                if (__left_acc && left) {
                    left_acc_policy<>::update(node);
                }
    
                if (__right_acc && right) {
                    right_acc_policy<>::update(node);
                }
    
                rbtree::link(node, parent, link);
                rbtree::insert(node);
    
                return elm;
            }
    
            type_t* replace(type_t *maped, type_t *new_elm) {
                rbtree::node *maped_node = &(maped->*__field);
                rbtree::node *new_node = &(new_elm->*__field);
                if (__left_acc && left_acc_policy<>::get() == maped_node) {
                    left_acc_policy<>::update(new_node);
                }
                if (__right_acc && right_acc_policy<>::get() == maped_node) {
                    right_acc_policy<>::update(maped_node);
                }
    
                rbtree::replace(maped_node, new_node);
                return maped;
            }
    
            type_t* replace(type_t *new_elm) {
                key_t key = getkey_t::get_key(new_elm);
                rbtree::node **link = &_root;
                rbtree::node *parent = nullptr;
                int left __attribute__((unused)) = 1;
                int right __attribute__((unused)) = 1;
                int n;
                type_t* elm;
    
                while (*link) {
                    parent = *link;
    
                    elm = static_cast<type_t*>(containerof_member(static_cast<entry_t*>(parent), __field));
                    n = compare_t::compare(key, getkey_t::get_key(elm));
    
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
    
                rbtree::node *node = &(new_elm->*__field);
    
                if (__left_acc && left) {
                    left_acc_policy<>::update(node);
                }
    
                if (__right_acc && right) {
                    right_acc_policy<>::update(node);
                }
    
                rbtree::link(node, parent, link);
                rbtree::insert(node);
    
                return nullptr;
            }
    
            type_t* remove(type_t *elm) {
                rbtree::node *node = &(elm->*__field);
    
                if (__left_acc && left_acc_policy<>::get() == node) {
                    left_acc_policy<>::update(node->next());
                }
                if (__right_acc && right_acc_policy<>::get() == node) {
                    right_acc_policy<>::update(node->prev());
                }
                rbtree::remove(node);
                return elm;
            }
    
            type_t* remove(key_t key) {
                type_t* elm = get(key);
                if (elm) {
                    remove(elm);
                }
                return elm;
            }
    
            iterator begin() {
                rbtree::node *node;
                if (__left_acc) {
                    node = left_acc_policy<>::get();
                }
                else {
                    node = rbtree::front();
                }
                return iterator(node);
            }
    
            const iterator begin() const {
                rbtree::node *node;
                if (__left_acc) {
                    node = left_acc_policy<>::get();
                }
                else {
                    node = rbtree::front();
                }
                return iterator(node);
            }
    
            iterator end() {
                return iterator(nullptr);
            }
    
            const iterator end() const {
                return iterator(nullptr);
            }
    
            iterator rbegin() {
                rbtree::node *node;
                if (__right_acc) {
                    node = right_acc_policy<>::get();
                }
                else {
                    node = rbtree::back();
                }
                return iterator(node);
            }
    
            const iterator rbegin() const {
                rbtree::node *node;
                if (__right_acc) {
                    node = right_acc_policy<>::get();
                }
                else {
                    node = rbtree::back();
                }
                return iterator(node);
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
    bool __left_acc = true, 
    bool __right_acc = false, 
    typename _Factory = typename factory_of<_T>::type,
    typename _Compare = comparer<_Key>,
    typename _GetKey = _T>
class map : public map_helper::impl<_Key, _T, _Base, _Entry, __field, __left_acc, __right_acc, _Factory, _GetKey, _Compare>::map {
public:
    map() : map_helper::impl<_Key, _T, _Base, _Entry, __field, __left_acc, __right_acc, _Factory, _GetKey, _Compare>::map() {
    }
};

#define ll_map(k, T, entry, ...) ll::map<k, T, typeof_container(&T::entry), typeof_member(&T::entry), &T::entry, ##__VA_ARGS__>

}

#endif
