#include <iostream>
using std::cout;
using std::endl;

#include <time.h>
#include <cstdio>

#include "libll++/memory.h"
#include "libll++/log.h"
#include "libll++/config_file.h"
#include "libll++/timeval.h"

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
    /*
    ll::obstack *pool = ll::_new<ll::obstack>();
    struct ::timeval tv;
    struct ::tm tm;
    ::gettimeofday(&tv, nullptr);
    ::gmtime_r(&tv.tv_sec, &tm);
    pool->print("%02d:%02d:%02d.%02d",
                tm.tm_hour, tm.tm_min, tm.tm_sec, 
                (int)ll::time_prec_msec::to_precval(tv.tv_usec));
    pool->grow("ab", 2);
    pool->finish();
    pool->clear();
    pool->print("%02d:%02d:%02d.%02d",
                tm.tm_hour, tm.tm_min, tm.tm_sec, 
                (int)ll::time_prec_msec::to_precval(tv.tv_usec));
    pool->grow("ab", 2);
    pool->finish();
    return 0;*/
    if (argc < 2) {
        return 0;
    }
    ll::config_doc doc;
    char *msg;
    int n = doc.load(argv[1], &msg);
    if (ll_failed(n)) {
        cout << "failed" << endl;
        cout << msg << endl;
        return n;
    }
    dump(doc);
    return 0;
}


