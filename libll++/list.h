#ifndef __LIBLLPP_LIST_H__
#define __LIBLLPP_LIST_H__

#include "member.h"

namespace ll {

#define __LIST_OBJECT__(o) static_cast<type*>(o)
#define __LIST_ENTRY__(x) static_cast<entry_type&>(__LIST_OBJECT__(x)->*(__field))

struct list_type {
    enum {
        list,
        clist,
        slist,
        stlist,
        unknown,
    };
};

template <typename _T, typename _Entry, _Entry _T::*__field, int __type = _Entry::type> 
struct list {
    static_assert(__type >= 0 && __type < list_type::unknown,
                  "unknown list type.");
};

/* list */
struct list_entry {
    static const int type = list_type::list;

    void *_next;
    void **_prev;
};

template <typename _T, typename _Entry, _Entry _T::*__field> 
struct list <_T, _Entry, __field, list_type::list> {
private:
    typedef _T type;
    typedef _Entry entry_type;

    type *_first;
    
public:
    class iterator {
    private:
        type *_ptr;
    public:
        iterator(type *object = nullptr) : _ptr(object) {}
        type& operator*() {
            return *_ptr;
        }

        type* operator->() {
            return _ptr;
        }

        type* pointer() {
            return _ptr;
        }

        iterator& operator++() {
            _ptr = __LIST_OBJECT__(__LIST_ENTRY__(_ptr)._next);
            return *this;
        }

        iterator operator++(int) {
            iterator tmp(_ptr);
            _ptr = __LIST_OBJECT__(__LIST_ENTRY__(_ptr)._next);
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
    list() : _first(nullptr) {}

    static type *insert_back(type *listelm, type *elm) {
        register entry_type &listed = __LIST_ENTRY__(listelm);
        register entry_type &entry = __LIST_ENTRY__(elm);

        if ((entry._next = listed._next) != nullptr) {
            __LIST_ENTRY__(listed._next)._prev = &entry._next;
        }
        listed._next = elm;
        entry._prev = &listed._next;
        return elm;
    }

    static type *insert_front(type *listelm, type *elm) {
        register entry_type &listed = __LIST_ENTRY__(listelm);
        register entry_type &entry = __LIST_ENTRY__(elm);

        entry._prev = listed._prev;
        entry._next = listelm;
        *listed._prev = elm;
        listed._prev = &entry._next;

        return elm;
    }

    static type *remove(type *elm) {
        register entry_type &entry = __LIST_ENTRY__(elm);

        if (entry._next != nullptr) {
            __LIST_ENTRY__(entry._next)._prev = entry._prev;
        }
        *entry._prev = entry._next;
        return elm;
    }

    type *first() {
        return _first;
    }

    type *front() {
        return _first;
    }

    type *next(type *elm) {
        return __LIST_OBJECT__(__LIST_ENTRY__(elm)._next);
    }

    bool empty() {
        return !_first;
    }

    void truncate() {
        _first = nullptr;
    }

    type *push_front(type *elm) {
        register entry_type &entry = __LIST_ENTRY__(elm);
        if ((entry._next = _first) != nullptr) {
            __LIST_ENTRY__(_first)._prev = &entry._next;
        }
        _first = elm;
        entry._prev = reinterpret_cast<void**>(&_first);
        return elm;
    }

    type *pop_front() {
        if (!_first) {
            return nullptr;
        }
        return remove(_first);
    }

    iterator begin() {
        return iterator(_first);
    }

    const iterator begin() const {
        return iterator(_first);
    }

    iterator end() {
        return iterator(nullptr);
    }

    const iterator end() const {
        return iterator(nullptr);
    }
};

/* clist */
struct clist_entry {
    static const int type = list_type::clist;
    clist_entry() {}
    clist_entry(clist_entry *next, clist_entry *prev): _next(next), _prev(prev) {}
    clist_entry *_next;
    clist_entry *_prev;

    void remove() {
        _next->_prev = _prev;
        _prev->_next = _next;
    }

    void insert_back(clist_entry *listed) {
        _next = listed->_next;
        _prev = listed;
        listed->_next->_prev = this;
        listed->_next = this;
    }

    void insert_front(clist_entry *listed) {
        _next = listed;
        _prev = listed->_prev;
        listed->_prev->_next = this;
        listed->_prev = this;
    }
};

#define __CLIST_OBJECT__(entry) ll::containerof_member(entry, __field)
template <typename _T, typename _Entry, _Entry _T::*__field> 
struct list <_T, _Entry, __field, list_type::clist>: protected clist_entry {
public:
    typedef _T type;
    typedef _Entry entry_type;

    class iterator {
    private:
        entry_type *_entry;
    public:
        iterator(entry_type *entry) : _entry(entry) {}
        type& operator*() {
            return *__CLIST_OBJECT__(_entry);
        }

        type* operator->() {
            return __CLIST_OBJECT__(_entry);
        }

        type* pointer() {
            return __CLIST_OBJECT__(_entry);
        }

        iterator& operator++() {
            _entry = _entry->_next;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp(_entry);
            _entry = _entry->_next;
            return tmp;
        }

        iterator& operator--() {
            _entry = _entry->_prev;
            return *this;
        }

        iterator operator--(int) {
            iterator tmp(_entry);
            _entry = _entry->_prev;
            return tmp;
        }

        bool operator==(const iterator &other) const {
            return _entry == other._entry;
        }

        bool operator!=(const iterator &other) const {
            return _entry != other._entry;
        }
    };

public:
    list() : clist_entry(this, this) {}
    
    static type *insert_back(type *listelm, type *elm) {
        register entry_type *listed = &(listelm->*__field);
        register entry_type *entry = &(elm->*__field);
        entry->insert_back(listed);
        return elm;
    }

    static type *insert_front(type *listelm, type *elm) {
        register entry_type *listed = &(listelm->*__field);
        register entry_type *entry = &(elm->*__field);
        entry->insert_front(listed);
        return elm;
    }

    static type *remove(type *elm) {
        register entry_type *entry = &(elm->*__field);
        entry->remove();
        return elm;
    }

    void truncate() {
        _next = this;
        _prev = this;
    }

    type *push_front(type *elm) {
        register entry_type *entry = &(elm->*__field);
        entry->insert_back(this);
        return elm;
    }

    type *push_back(type *elm) {
        register entry_type *entry = &(elm->*__field);
        entry->insert_front(this);
        return elm;
    }

    type *pop_front() {
        register entry_type *entry = _next;
        if (entry == this) {
            return nullptr;
        }
        else {
            entry->remove();
            return __CLIST_OBJECT__(entry);
        }
    }

    type *pop_back() {
        register entry_type *entry = _prev;
        if (entry == this) {
            return nullptr;
        }
        else {
            entry->remove();
            return __CLIST_OBJECT__(entry);
        }
    }

    type *first() {
        return _next == this ? nullptr : __CLIST_OBJECT__(_next);
    }

    type *last() {
        return _prev == this ? nullptr : __CLIST_OBJECT__(_prev);
    }

    type *front() {
        return _next == this ? nullptr : __CLIST_OBJECT__(_next);
    }

    type *back() {
        return _prev == this ? nullptr : __CLIST_OBJECT__(_prev);
    }

    bool empty() {
        return _prev == _next;
    }

    iterator begin() {
        return iterator(_next);
    }

    const iterator begin() const {
        return iterator(_next);
    }

    iterator end() {
        return iterator(this);
    }

    const iterator end() const {
        return iterator(this);
    }
};

/* slist_entry */
struct slist_entry {
    static const int type = list_type::slist;

    void *_next;
};

template <typename _T, typename _Entry, _Entry _T::*__field> 
struct list <_T, _Entry, __field, list_type::slist> {
private:
    typedef _T type;
    typedef _Entry entry_type;
    type *_first;

public:
    class iterator {
    private:
        type *_ptr;
    public:
        iterator(type *object) : _ptr(object) {}
        type& operator*() {
            return *_ptr;
        }

        type* operator->() {
            return _ptr;
        }

        type* pointer() {
            return _ptr;
        }

        iterator& operator++() {
            _ptr = __LIST_OBJECT__(__LIST_ENTRY__(_ptr)._next);
            return *this;
        }

        iterator operator++(int) {
            iterator tmp(_ptr);
            _ptr = __LIST_OBJECT__(__LIST_ENTRY__(_ptr)._next);
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
    list() : _first(nullptr) {}

    static type *insert_back(type *listelm, type *elm) {
        register entry_type &listed = __LIST_ENTRY__(listelm);
        register entry_type &entry = __LIST_ENTRY__(elm);

        entry._next = listed._next;
        listed._next = elm;
        return elm;
    }

    bool empty() {
        return !_first;
    }

    void truncate() {
        _first = nullptr;
    }

    type *first() {
        return _first;
    }

    type *front() {
        return _first;
    }

    type *next(type *elm) {
        return __LIST_OBJECT__(__LIST_ENTRY__(elm)._next);
    }

    type *push_front(type *elm) {
        register entry_type &entry = __LIST_ENTRY__(elm);

        entry._next = _first;
        _first = elm;
        return elm;
    }

    type *pop_front() {
        type *elm = _first;
        if (elm) {
            _first = __LIST_OBJECT__(__LIST_ENTRY__(elm)._next);
        }
        return elm;
    }

    type *remove(type *elm, type *prev) {
        if (!prev) {
            return pop_front();
        }
        else {
            register entry_type &entry = __LIST_ENTRY__(prev);
            entry._next = __LIST_ENTRY__(entry._next)._next;
            return elm;
        }
    }

    type *remove(type *elm) {
        if (_first == elm) {
            return pop_front();
        }
        else {
            type *curelm = _first;
            register entry_type *entry;
            while (curelm) {
                entry = &__LIST_ENTRY__(curelm);
                if (entry->_next == elm) {
                    break;
                }
                curelm = __LIST_OBJECT__(entry->_next);
            }
            if (curelm) {
                entry->_next = __LIST_ENTRY__(entry->_next)._next;
                return elm;
            }
            return nullptr;
        }
    }

    iterator begin() {
        return iterator(_first);
    }

    const iterator begin() const {
        return iterator(_first);
    }

    iterator end() {
        return iterator(nullptr);
    }

    const iterator end() const {
        return iterator(nullptr);
    }
};

/* stlist */
struct stlist_entry {
    static const int type = list_type::stlist;

    void *_next;
};

template <typename _T, typename _Entry, _Entry _T::*__field> 
struct list <_T, _Entry, __field, list_type::stlist> {
private:
    typedef _T type;
    typedef _Entry entry_type;

    type *_first;
    type **_last;
public:
    class iterator {
    private:
        type *_ptr;
    public:
        iterator(type *object) : _ptr(object) {}

        type& operator*() {
            return *_ptr;
        }

        type* operator->() {
            return _ptr;
        }

        type* pointer() {
            return _ptr;
        }

        iterator& operator++() {
            _ptr = __LIST_OBJECT__(__LIST_ENTRY__(_ptr)._next);
            return *this;
        }

        iterator operator++(int) {
            iterator tmp(_ptr);
            _ptr = __LIST_OBJECT__(__LIST_ENTRY__(_ptr)._next);
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
    list() : _first(nullptr), _last(&_first) {}

    bool empty() {
        return !_first;
    }

    type *first() {
        return _first;
    }

    type *front() {
        return _first;
    }

    type *next(type *elm) {
        return __LIST_OBJECT__(__LIST_ENTRY__(elm)._next);
    }

    void truncate() {
        _first = nullptr;
        _last = &_first;
    }

    type *push_front(type *elm) {
        register entry_type &entry = __LIST_ENTRY__(elm);

        if ((entry._next = _first) == nullptr) {
            _last = reinterpret_cast<type**>(&entry._next);
        }
        _first = elm;
        return elm;
    }

    type *push_back(type *elm) {
        register entry_type &entry = __LIST_ENTRY__(elm);

        entry._next = nullptr;
        *_last = elm;
        _last = reinterpret_cast<type**>(&entry._next);

        return elm;
    }

    type* pop_front() {
        type* elm = _first;
        if (elm) {
            if ((_first = __LIST_OBJECT__(__LIST_ENTRY__(elm)._next)) == nullptr) {
                _last = &_first;
            }
        }
        return elm;
    }

    type *insert_back(type *listelm, type *elm) {
        register entry_type &listed = __LIST_ENTRY__(listelm);
        register entry_type &entry = __LIST_ENTRY__(elm);

        if ((entry._next = listed._next) == nullptr) {
            _last = &entry._next;
        }
        listed._next = elm;
        return elm;
    }

    type *remove(type *elm, type *prev) {
        if (!prev) {
            return pop_front(elm);
        } 
        else {
            register entry_type &entry = __LIST_ENTRY__(prev);
            if (LL_ISnullptr(entry._next = __LIST_ENTRY__(entry._next)._next)) {
                _last = &entry._next;
            }
            return elm;
        }
    }

    type *remove(type *elm) {
        if (_first == elm) {
            return pop_front();
        } 
        else {
            type *curelm = _first;
            register entry_type *entry;
            while (curelm) {
                entry = &__LIST_ENTRY__(curelm);
                if (entry->_next == elm) {
                    break;
                }
                curelm = __LIST_OBJECT__(entry->_next);
            }
            if (curelm) {
                if (!(entry->_next = __LIST_ENTRY__(entry->_next)._next)) {
                    _last = reinterpret_cast<type**>(&entry->_next);
                }
                return elm;
            }
            return nullptr;
        }
    }

    iterator begin() {
        return iterator(_first);
    }

    const iterator begin() const {
        return iterator(_first);
    }

    iterator end() {
        return iterator(nullptr);
    }

    const iterator end() const {
        return iterator(nullptr);
    }
};

#define list(_T, entry) ll::list<typeof_container(&_T::entry), typeof_member(&_T::entry), &_T::entry>
//#define list(_T, entry) ll::list<_T, decltype(_T::entry), &_T::entry>
};
#endif

