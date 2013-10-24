#include <iostream>
using std::cout;
using std::endl;

#include "libll++/config_file.h"

void dump(ll::config_items &items, int indent)
{
    for (auto &item : items) {
        if (item.children.empty()) {
            for (int i = 0; i < indent * 4; i++) {
                cout << ' ';
            }
            cout << (const char*)item;
            for (unsigned i = 0; i < item; i++) {
                cout << " " << (const char*)item[i];
            }
            cout << endl;
        }
        else {
            for (int i = 0; i < indent * 4; i++) {
                cout << ' ';
            }
            cout << "<" << (const char*)item;
            for (unsigned i = 0; i < item; i++) {
                cout << " " << (const char*)item[i];
            }
            cout << ">" << endl;
            dump(item.children, indent + 1);
        }
    }
}

void dump(ll::config_doc &doc)
{
    cout << "========================================" << endl;
    dump(doc.items, 0);
    cout << "========================================" << endl;
}

struct foo {
    ll::clist_entry entry;
    int n;
    foo(int m) : n(m) {}
};

int main(int argc, char **argv)
{
    ll_list(foo, entry) list;
    list.push_back(new foo(1));
    for (auto &x : list) {
        cout << x.n << endl;
    }

    if (argc < 2) {
        return 0;
    }
    ll::config_doc doc;
    ll_failed_return(doc.load(argv[1]));
    dump(doc);
    return 0;
}


