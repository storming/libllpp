#include <iostream>

using std::cout;
using std::endl;

#include "libll++/map.h"

struct foo {
    ll::map_entry _entry;
    unsigned _key;
    unsigned _flag;
    foo(unsigned key) : _key(key), _flag() {}
    foo(unsigned key, unsigned flag) : _key(key), _flag(flag) {}

    static unsigned get_key(foo *f) {
        return f->_key;
    }
};

struct foo2 {

};

struct foo3 : foo2 {
    virtual int kaka() {
        return 0;
    }
    virtual int kaka2() {
        return 0;
    }
};

struct foo4 : foo3 {
    int kaka() {
        return 0;
    }
    int kaka2() {
        return 0;
    }
};
int main()
{
    ll_map(unsigned, foo, _entry) map;
    for (unsigned i = 0; i < 10; i++) {
        map.probe(i, nullptr, ll::pool::global());
    }

    for (unsigned i = 0; i < 10; i++) {
        assert(map.get(i)->_key == i);
    }

    cout << map.front()->_key << endl;
    cout << map.back()->_key << endl;

    cout << "====" << endl;

    for (auto &obj : map) {
        cout << obj._key << endl;
    }

    cout << "====" << endl;
    map.insert<>(ll::_new<foo>(ll::pool::global(), 5, 1));
    foo *elm9 = map.insert<false>(ll::_new<foo>(ll::pool::global(), 9, 1));

    for (auto &obj : map) {
        cout << obj._key << " " << obj._flag << endl;
    }

    cout << "====" << endl;
    map.replace(map.get(4), ll::_new<foo>(ll::pool::global(), 4, 1));
    map.replace(ll::_new<foo>(ll::pool::global(), 6, 1));
    for (auto &obj : map) {
        cout << obj._key << " " << obj._flag << endl;
    }

    cout << "====" << endl;
    map.remove((unsigned)0);
    map.remove(elm9);
    for (auto &obj : map) {
        cout << obj._key << " " << obj._flag << endl;
    }

    cout << "====" << endl;
    cout << map.front()->_key << endl;
    cout << map.back()->_key << endl;

    cout << "====" << endl;
    cout << sizeof(foo3) << endl;
    cout << sizeof(foo4) << endl;

    return 0;
}
