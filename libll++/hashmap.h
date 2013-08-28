#ifndef __LIBLLPP_HASHMAP_H__
#define __LIBLLPP_HASHMAP_H__

#include "list.h"
#include "hash.h"
#include "memory.h"
#include "bitorder.h"
#include "compare.h"

namespace ll {

namespace hashmap_helper { // begin namespace hashmap_helper
    struct cache_hash_entry {
        static constexpr bool cache_hash = true;
        unsigned _hash;
    };

    struct nocache_hash_entry {
        static constexpr bool cache_hash = false;
    };

    struct fast_remove_entry: public list_entry {
        static constexpr bool fast_remove = true;
    };

    struct slow_remove_entry: public slist_entry {
        static constexpr bool fast_remove = false;
    };

    template <    
        typename _Key,
        typename _T,
        typename _Base,
        typename _Entry,
        _Entry _Base::*__field,
        typename _Factory,
        typename _Allocator,
        typename _Hash,
        typename _Compare,
        typename _GetKey>
    struct impl {
        static constexpr unsigned min_order = 3;
        static constexpr unsigned max_order = 20;

        static constexpr unsigned min_capacity = (1 << min_order);
        static constexpr unsigned max_capacity = (1 << max_order);

        typedef _T                                  type_t;
        typedef _Key                                key_t;
        typedef _Base                               base_t;
        typedef _Entry                              entry_t;
        typedef _Factory                            factory_t;
        typedef typename factory_t::allocator_t     factory_allocator_t;
        typedef _Allocator                          allocator_t;
        typedef list<base_t, entry_t, __field>      list_t;
        typedef typename list_t::iterator           list_iterator_t;
        typedef _Hash                               hash_make_t;
        typedef _Compare                            compare_t;
        typedef _GetKey                             getkey_t;
        class map;

        template <typename _AEntry = entry_t, bool = _AEntry::cache_hash>
        struct get_hash_policy {
            static unsigned get_hash(type_t *elm) {
                return (elm->*__field)._hash;
            }
        };

        template <typename _AEntry>
        struct get_hash_policy<_AEntry, false> {
            static unsigned get_hash(type_t *elm) {
                key_t key = getkey_t::get_key(elm);
                return hash_make_t::make(key);
            }
        };

        template <typename _AEntry = entry_t, bool = _AEntry::cache_hash>
        struct set_hash_policy {
            static void set_hash(type_t *elm, unsigned hash) {
                (elm->*__field)._hash = hash;
            }
        };

        template <typename _AEntry>
        struct set_hash_policy<_AEntry, false> {
            static void set_hash(type_t *elm, unsigned hash) {
            }
        };

        template <typename _AEntry = entry_t, bool = _AEntry::cache_hash>
        struct compare_policy {
            static bool compare(type_t *elm, key_t key, unsigned hash) {
                return hash == (elm->*__field)._hash && !compare_t::compare(key, getkey_t::get_key(elm));
            }
        };

        template <typename _AEntry>
        struct compare_policy<_AEntry, false> {
            static bool compare(type_t *elm, key_t key, unsigned hash) {
                return !compare_t::compare(key, getkey_t::get_key(elm));
            }
        };

        template <typename _AEntry = entry_t, bool = _AEntry::cache_hash>
        struct create_policy {
            template <typename ...Args>
            static type_t *create(unsigned hash, key_t key, factory_allocator_t *allocator, Args &&... args) {
                type_t *elm = ll::create<type_t, factory_t>(allocator, key, std::forward<Args>(args)...);
                (elm->*__field)._hash = hash;
                return elm;
            }
        };

        template <typename _AEntry>
        struct create_policy<_AEntry, false> {
            template <typename ...Args>
            static type_t *create(unsigned hash, key_t key, factory_allocator_t *allocator, Args &&... args) {
                return ll::create<type_t, factory_t>(allocator, key, std::forward<Args>(args)...);
            }
        };

        template <typename _AEntry = entry_t, bool = _AEntry::fast_remove>
        struct remove_policy {
            static type_t *remove(map *m, key_t key) {
                type_t *elm = m->get(key);
                if (elm) {
                    m->_count--;
                    return static_cast<type_t*>(list_t::remove(elm));
                } 
                return nullptr;
            }

            static type_t *remove(map *map, type_t *elm) {
                map->_count--;
                return static_cast<type_t*>(list_t::remove(elm));
            }
        };

        template <typename _AEntry>
        struct remove_policy<_AEntry, false> {
            static type_t *remove(map *map, key_t key) {
                unsigned hash = hash_make_t::make(key);
                unsigned slot = hash & map->_capacity;
                list_t *list = map->_array + slot;
                type_t *prev = nullptr;
                type_t *elm = static_cast<type_t*>(list->front());
                while (elm) {
                    if (compare_policy<>::compare(elm, key, hash)) {
                        map->_count--;
                        return static_cast<type_t*>(list->remove(elm, prev));
                    }
                    prev = elm;
                    elm = static_cast<type_t*>(list_t::next(elm));
                }
                return nullptr;
            }

            static type_t *remove(map *map, type_t *elm) {
                unsigned slot = get_hash_policy<>::get_hash(elm) & map->_capacity;
                list_t *list = map->_array + slot;
                if ((elm = static_cast<type_t*>(list->remove(elm)))) {
                    map->_count--;
                }
                return elm;
            }
        };

        template <typename _AEntry = entry_t, bool = _AEntry::fast_remove>
        struct replace_policy {
            static type_t *replace(map *map, key_t key, type_t *obj) {
                unsigned hash = hash_make_t::make(key);
                unsigned slot = hash & map->_capacity;
                list_t *list = map->_array + slot;
                type_t *elm = static_cast<type_t*>(list->front());

                while (elm) {
                    if (compare_policy<>::compare(elm, key, hash)) {
                        list_t::remove(elm);
                        break;
                    }
                    elm = static_cast<type_t*>(list_t::next(elm));
                }

                set_hash_policy<>::set_hash(obj, hash);
                list->push_front(obj);
                if (!elm) {
                    map->_count++;
                }
                return elm;
            }
        };

        template <typename _AEntry>
        struct replace_policy<_AEntry, false> {
            static type_t *replace(map *map, key_t key, type_t *obj) {
                unsigned hash = hash_make_t::make(key);
                unsigned slot = hash & map->_capacity;
                list_t *list = map->_array + slot;
                type_t *elm = static_cast<type_t*>(list->front());
                type_t *prev = nullptr;

                while (elm) {
                    if (compare_policy<>::compare(elm, key, hash)) {
                        list->remove(elm, prev);
                        break;
                    }
                    prev = elm;
                    elm = static_cast<type_t*>(list_t::next(elm));
                }

                set_hash_policy<>::set_hash(obj, hash);
                list->push_front(obj);
                if (!elm) {
                    map->_count++;
                }
                return elm;
            }
        };

        struct allocator_policy_base {
            static list_t *alloc_array(allocator_t *allocator, unsigned size) {
                size = sizeof(list_t) * (size + 1);
                list_t *array = (list_t*)allocator->alloc(size);
                memset(array, 0, size);
                return array;
            }

            static void free_array(allocator_t *allocator, list_t *array, unsigned size) {
                size = sizeof(list_t) * (size + 1);
                allocator->free(array, size);
            }
        };

        template <typename _AAllocator = allocator_t, bool = check_free<_AAllocator>::value>
        struct allocator_policy: allocator_policy_base {
            allocator_t *_allocator;
            list_t *_array;
            unsigned _capacity;
            allocator_policy(unsigned capacity, allocator_t *allocator) : 
                _allocator(allocator), 
                _array(allocator_policy_base::alloc_array(allocator, capacity)),
                _capacity(capacity) {}
            ~allocator_policy() {
                allocator_policy_base::free_array(_allocator, _array, _capacity);
            }

            bool expand() {
                unsigned new_size = (_capacity << 1) + 1;
                if (new_size >= max_capacity) {
                    return false;
                }

                list_t *new_map = allocator_policy_base::alloc_array(_allocator, new_size);
                list_t *list, *new_list;
                type_t *elm;
                list = _array;
                for (unsigned i = 0; i <= _capacity; i++, list++) {
                    while ((elm = static_cast<type_t*>(list->pop_front()))) {
                        new_list = new_map + (get_hash_policy<>::get_hash(elm) & _capacity);
                        new_list->push_front(elm);
                    }
                }
                allocator_policy_base::free_array(_allocator, _array, _capacity);
                _array = new_map;
                _capacity = new_size;
                return true;
            }
        };

        template <typename _AAllocator>
        struct allocator_policy<_AAllocator, false> : allocator_policy_base {
            list_t *_array;
            unsigned _capacity;
            allocator_policy(unsigned capacity, allocator_t *allocator) : 
                _array(allocator_policy_base::alloc_array(allocator, capacity)),
                _capacity(capacity) {}
            bool expand() {
                return false;
            }
        };

        class map : protected allocator_policy<> {
            friend class remove_policy<>;
            friend class replace_policy<>;
            friend class iterator;
        protected:
            using allocator_policy<>::_array;
            using allocator_policy<>::_capacity;
            unsigned _count;

            unsigned calc_initsize(unsigned initsize) {
                if (!ll_is_p2(initsize)) {
                    unsigned order = bitorder::order(initsize);
                    if (order > max_order) {
                        order = max_order;
                    }
                    else if (order < min_order) {
                        order = min_order;
                    }
                    initsize = (1 << order);
                }
                else {
                    if (initsize < min_capacity) {
                        initsize = min_capacity;
                    }
                    else if (initsize > max_capacity) {
                        initsize = max_capacity;
                    }
                }
                return initsize - 1;
            }
        public:
            class iterator {
                friend class map;
            protected:
                map *_map;
                list_t *_list;
                type_t *_ptr;

                iterator(type_t *ptr) : _ptr(ptr) {}
                iterator(map *map) : _map(map), _ptr() {
                    list_t *bound = map->_array + map->_capacity;
                    for (_list = map->_array; _list <= bound; _list++) {
                        if ((_ptr = static_cast<type_t*>(_list->front()))) {
                            break;
                        }
                    }
                }

                type_t *next(type_t *elm) {
                    if (!elm) {
                        return nullptr;
                    }

                    if ((elm = static_cast<type_t*>(list_t::next(elm)))) {
                        return elm;
                    }

                    list_t *bound = _map->_array + _map->_capacity;
                    for (_list++; _list <= bound; _list++) {
                        if ((elm = static_cast<type_t*>(_list->front()))) {
                            return elm;
                        }
                    }
                    return nullptr;
                }

            public:
                type_t& operator*() {
                    return *_ptr;
                }

                type_t* operator->() {
                    return _ptr;
                }

                type_t* pointer() {
                    return _ptr;
                }

                iterator& operator++() {
                    _ptr = next(_ptr);
                    return *this;
                }

                iterator operator++(int) {
                    iterator tmp = *this;
                    _ptr = next(_ptr);
                    return tmp;
                }

                bool operator==(const iterator &other) const {
                    return _ptr == other._ptr;
                }

                bool operator!=(const iterator &other) const {
                    return _ptr != other._ptr;
                }
            };

        public:
            map(unsigned initsize, allocator_t *allocator) : 
                allocator_policy<>(calc_initsize(initsize), allocator), _count(0) {}

            ~map() {}

            using allocator_policy<>::expand;

            unsigned count() {
                return _count;
            }

            unsigned capacity() {
                return _capacity;
            }

            void expand_if() {
                if (_count >= _capacity) {
                    expand();
                }
            }

            void clear(factory_allocator_t *allocator) {
                list_t *list = _array;
                type_t *elm;
                for (unsigned i = 0; i <= _capacity; i++, list++) {
                    while ((elm = static_cast<type_t*>(list->pop_front()))) {
                        destroy<type_t, factory_t>(elm, allocator);
                    }
                }
                _count = 0;
            }

            void truncate() {
                memset(_array, 0, sizeof(list_t) * (_capacity + 1));
                _count = 0;
            }

            type_t *get(key_t key) {
                unsigned hash = hash_make_t::make(key);
                unsigned slot = hash & _capacity;
                list_t *list = _array + slot;

                type_t *elm = static_cast<type_t*>(list->front());
                while (elm) {
                    if (compare_policy<>::compare(elm, key, hash)) {
                        break;
                    }
                    elm = static_cast<type_t*>(list_t::next(elm));
                }
                return elm;
            }

            template <typename ...Args>
            type_t *probe(key_t key, int *flag, factory_allocator_t *allocator, Args &&... args) {
                unsigned hash = hash_make_t::make(key);
                unsigned slot = hash & _capacity;
                list_t *list = _array + slot;

                type_t *elm = static_cast<type_t*>(list->front());
                while (elm) {
                    if (compare_policy<>::compare(elm, key, hash)) {
                        if (flag) {
                            *flag = 0;
                        }
                        return elm;
                    }
                    elm = static_cast<type_t*>(list_t::next(elm));
                }

                elm = create_policy<>::create(hash, key, allocator, std::forward<Args>(args)...);
                list->push_front(elm);
                _count++;
                if (flag) {
                    *flag = 1;
                }
                return elm;
            }

            type_t *remove(key_t key) {
                return remove_policy<>::remove(this, key);
            }

            type_t *remove(type_t *elm) {
                return remove_policy<>::remove(this, elm);
            }

            type_t* replace(key_t key, type_t *obj) {
                return replace_policy<>::replace(this, key, obj);
            }

            iterator begin() {
                return iterator(this);
            }

            const iterator begin() const {
                return iterator(this);
            }

            iterator end() {
                return iterator(static_cast<type_t*>(nullptr));
            }

            const iterator end() const {
                return iterator(static_cast<type_t*>(nullptr));
            }

            double degree_of_uniformity() {
                unsigned n = 0;
                list_t *list = _array;
                for (unsigned i = 0; i <= _capacity; i++, list++) {
                    if (!list->empty()) {
                        n++;
                    }
                }
                return (double)n / (double)(_capacity + 1);
            }
        };
    };
} // end namespace hashmap_helper

template <bool __fast_remove = true, bool __cache_hash = true>
struct hashmap_entry : hashmap_helper::fast_remove_entry, hashmap_helper::cache_hash_entry {};

template <>
struct hashmap_entry<true, false> : hashmap_helper::fast_remove_entry, hashmap_helper::nocache_hash_entry{};

template <>
struct hashmap_entry<false, true> : hashmap_helper::slow_remove_entry, hashmap_helper::cache_hash_entry {};

template <>
struct hashmap_entry<false, false> : hashmap_helper::slow_remove_entry, hashmap_helper::nocache_hash_entry {};

template <
    typename _Key,
    typename _T,
    typename _Base,
    typename _Entry,
    _Entry _Base::*__field,
    typename _Allocator = pool,
    typename _Factory = typename factory_of<_T>::type,
    typename _Hash = hash<_Key>,
    typename _Compare = equal_compare<_Key>,
    typename _GetKey = _T>
class hashmap: public hashmap_helper::impl<_Key, _T, _Base, _Entry, __field, _Factory, _Allocator, _Hash, _Compare, _GetKey>::map {
public:
    hashmap(_Allocator *allocator, unsigned initsize = 32) : 
        hashmap_helper::impl<_Key, _T, _Base, _Entry, __field, _Factory, _Allocator, _Hash, _Compare, _GetKey>::map(initsize, allocator) {}
};

#define ll_hashmap(k, T, entry, ...) ll::hashmap<k, T, typeof_container(&T::entry), typeof_member(&T::entry), &T::entry, ##__VA_ARGS__>

}

#endif
