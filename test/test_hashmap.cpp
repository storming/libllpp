#include <iostream>
using std::cout;
using std::endl;

#include <cstdio>
#include <cstdlib>

#include "libll++/hashmap.h"
#include "libll++/memory.h"
#include "libll++/crc.h"
#include "libll++/time.h"

struct foo {
    unsigned _key;
    foo(unsigned key) : _key(key) {}

    ll::hashmap_entry<> fr_c_entry;
    ll::hashmap_entry<true, false> fr_nc_entry;
    ll::hashmap_entry<false, true> sr_c_entry;
    ll::hashmap_entry<false, false> sr_nc_entry;

    static unsigned get_key(foo *f) {
        return f->_key;
    }
};

struct A {
    int n;
};

struct B : A, foo {
    B(unsigned key) : foo(key) {}
};

using foo2 = ll::factory_bind<B, ll::factory<B, ll::pool>>;

template <typename _T>
struct factory_of {
    template<typename A, typename = void>
    struct got_type : std::false_type {};

    template<typename A>
    struct got_type<A> : std::true_type {
        typedef A type;
    };

    template<typename A, typename = std::true_type>
    struct checker {
        typedef ll::factory<_T, ll::pool> factory_t;
    };

    template<typename A>
    struct checker<A, std::integral_constant<bool, got_type<typename A::factory_t>::value>> {
        typedef typename A::factory_t factory_t;
    };

    typedef typename checker<_T>::factory_t type;
};

int main()
{
    #define COUNT 1000000UL

    do {
        ll::time_trace t;
        ll::timeval tv = t.check();
        ll_hashmap(unsigned, foo2, fr_c_entry) map(ll::pool::global(), 9000);
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();
        for (unsigned i = 0; i <= map.capacity(); i++) {
            map.probe(i, nullptr, ll::pool::global());
        }
        tv = t.check();
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();

        for (unsigned i = 0; i < COUNT; i++) {
            map.get(rand());
        }
        tv = t.check();
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();

        for (unsigned i = 0; i < COUNT; i++) {
            map.get(rand() & map.capacity());
        }
        tv = t.check();
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();

        assert(map.remove(100));
        assert(map.remove(map.get(101)));
        assert(map.replace(102, ll::create<foo2>(ll::pool::global(), 102)));
        map.clear(ll::pool::global());

        for (unsigned int i = 0; i < 20; i++) {
            map.probe(i, nullptr, ll::pool::global());
        }
        cout << map.count() << endl;
        for (auto &elm : map) {
            cout << "key: " << elm._key << endl;
        }
    } while (0);
    
    do {
        ll::time_trace t;
        ll::timeval tv = t.check();
        ll_hashmap(unsigned, foo2, fr_nc_entry) map(ll::pool::global(), 9000);
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();
        for (unsigned i = 0; i <= map.capacity(); i++) {
            map.probe(i, nullptr, ll::pool::global());
        }
        tv = t.check();
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();

        for (unsigned i = 0; i < COUNT; i++) {
            map.get(rand());
        }
        tv = t.check();
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();

        for (unsigned i = 0; i < COUNT; i++) {
            map.get(rand() & map.capacity());
        }
        tv = t.check();
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();

        assert(map.remove(100));
        assert(map.remove(map.get(101)));
        assert(map.replace(102, ll::create<foo2>(ll::pool::global(), 102)));
        map.clear(ll::pool::global());

        for (unsigned int i = 0; i < 20; i++) {
            map.probe(i, nullptr, ll::pool::global());
        }
        cout << map.count() << endl;
        for (auto &elm : map) {
            cout << "key: " << elm._key << endl;
        }
    } while (0);
    
    do {
        ll::time_trace t;
        ll::timeval tv = t.check();
        ll_hashmap(unsigned, foo2, sr_c_entry) map(ll::pool::global(), 9000);
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();
        for (unsigned i = 0; i <= map.capacity(); i++) {
            map.probe(i, nullptr, ll::pool::global());
        }
        tv = t.check();
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();

        for (unsigned i = 0; i < COUNT; i++) {
            map.get(rand());
        }
        tv = t.check();
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();

        for (unsigned i = 0; i < COUNT; i++) {
            map.get(rand() & map.capacity());
        }
        tv = t.check();
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();

        assert(map.remove(100));
        assert(map.remove(map.get(101)));
        assert(map.replace(102, ll::create<foo2>(ll::pool::global(), 102)));
        map.clear(ll::pool::global());

        for (unsigned int i = 0; i < 20; i++) {
            map.probe(i, nullptr, ll::pool::global());
        }
        cout << map.count() << endl;
        for (auto &elm : map) {
            cout << "key: " << elm._key << endl;
        }
    } while (0);

    do {
        ll::time_trace t;
        ll::timeval tv = t.check();
        ll_hashmap(unsigned, foo2, sr_nc_entry) map(ll::pool::global(), 9000);
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();
        for (unsigned i = 0; i <= map.capacity(); i++) {
            map.probe(i, nullptr, ll::pool::global());
        }
        tv = t.check();
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();

        for (unsigned i = 0; i < COUNT; i++) {
            map.get(rand());
        }
        tv = t.check();
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();

        for (unsigned i = 0; i < COUNT; i++) {
            map.get(rand() & map.capacity());
        }
        tv = t.check();
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();

        assert(map.remove(100));
        assert(map.remove(map.get(101)));
        assert(map.replace(102, ll::create<foo2>(ll::pool::global(), 102)));
        map.clear(ll::pool::global());

        for (unsigned int i = 0; i < 20; i++) {
            map.probe(i, nullptr, ll::pool::global());
        }
        cout << map.count() << endl;
        for (auto &elm : map) {
            cout << "key: " << elm._key << endl;
        }
    } while (0);

    do {
        ll::time_trace t;
        ll::timeval tv = t.check();
        ll_hashmap(unsigned, foo2, fr_c_entry, ll::caches) map(ll::caches::global());
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();
        for (unsigned i = 0; i <= map.capacity(); i++) {
            map.probe(i, nullptr, ll::pool::global());
        }
        tv = t.check();
        cout << "count=" << map.count() << ", capacity=" << map.capacity() << ", doi=" << map.degree_of_uniformity() * 100 << "%" << " time=" << tv << endl;
        t.check();

        map.expand_if();
        for (unsigned i = map.count(); i <= map.capacity(); i++) {
            map.probe(i, nullptr, ll::pool::global());
        }

        cout << map.count() << endl;
        for (auto &elm : map) {
            cout << "key: " << elm._key << endl;
        }
    } while (0);

    return 0;
}
