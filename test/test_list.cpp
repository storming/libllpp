#include <iostream>
#include "libll++/list.h"
using std::cout;
using std::endl;

struct A {
    ll::list_entry list_entry;
    ll::slist_entry slist_entry;
    ll::stlist_entry stlist_entry;
    ll::clist_entry clist_entry;

    int _value;
    A(int value) : _value(value) {}
};

template <typename _T>
void dump(_T &list) 
{
    for (auto it : list) {
        cout << it._value << " ";
    }
    cout << endl;
}

#define ELEMS 6
A *a[ELEMS];

void test_list() 
{
    cout << "== test list ==" << endl;
    cout << "push front: ";
    ll_list(A, list_entry) list;
    for (int i = 0; i < ELEMS; i++) {
        a[i] = list.push_front(new A(i));
    }
    dump(list);
    cout << "pop front: [" << list.pop_front()->_value << "] ";
    dump(list);
    cout << "remove 1 0: ";
    decltype(list)::remove(a[1]);
    decltype(list)::remove(a[0]);
    dump(list);
    cout << "truncate: ";
    list.init();
    dump(list);
}

void test_slist() 
{
    cout << "== test slist ==" << endl;
    cout << "push front: ";
    ll_list(A, slist_entry) list;
    for (int i = 0; i < ELEMS; i++) {
        a[i] = list.push_front(new A(i));
    }
    dump(list);
    cout << "pop front: [" << list.pop_front()->_value << "] ";
    dump(list);
    cout << "remove 1 0: ";
    list.remove(a[1]);
    list.remove(a[0]);
    dump(list);
    cout << "truncate: ";
    list.init();
    dump(list);
}

void test_stlist() 
{
    cout << "== test stlist ==" << endl;
    cout << "push front: ";
    ll_list(A, stlist_entry) list;
    for (int i = 0; i < ELEMS; i++) {
        a[i] = list.push_front(new A(i));
    }
    dump(list);
    cout << "pop front: [" << list.pop_front()->_value << "] ";
    dump(list);
    cout << "remove 1 0: ";
    list.remove(a[1]);
    list.remove(a[0]);
    dump(list);
    cout << "truncate: ";
    list.init();
    dump(list);

    cout << "push back: ";
    for (int i = 0; i < ELEMS; i++) {
        a[i] = list.push_back(new A(i));
    }
    dump(list);
}

void test_clist() 
{
    cout << "== test clist ==" << endl;
    cout << "push front: ";
    ll_list(A, clist_entry) list;
    for (int i = 0; i < ELEMS; i++) {
        a[i] = list.push_front(new A(i));
    }
    dump(list);
    cout << "pop front: [" << list.pop_front()->_value << "] ";
    dump(list);
    cout << "remove 1 0: ";
    list.remove(a[1]);
    list.remove(a[0]);
    dump(list);
    cout << "truncate: ";
    list.init();
    dump(list);

    cout << "push back: ";
    for (int i = 0; i < ELEMS; i++) {
        a[i] = list.push_back(new A(i));
    }
    dump(list);
}

int main() 
{
    test_list();
    test_slist();
    test_stlist();
    test_clist();
    return 0;
}
