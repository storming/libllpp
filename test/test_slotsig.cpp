#include <iostream>
#include <type_traits>
#include <utility>

using std::cout;
using std::endl;

#include "libll++/slotsig.h"
#include "libll++/memory.h"
#include "libll++/timeval.h"

struct foo {

    int _n;
    const char *_p;
    int operator()(int &magic) {
        cout << "bbbbbbbbbbb" << magic++ << endl;
        return 0;
    }

    int dodo(int magic) {
        cout << "kkkkkkkkkk" << magic++ << endl;
        return 0;
    }
    
    foo() {}
    foo(const char *p) : _p(p) {}
    foo(int n, const char *p) : _n(n), _p(p) {}

};

#define TREEBIN_SHIFT 8
#define NTREEBINS 32

#define compute_tree_index(S, I)\
{\
  unsigned int X = S >> TREEBIN_SHIFT;\
  if (X == 0)\
    I = 0;\
  else if (X > 0xFFFF)\
    I = NTREEBINS-1;\
  else {\
    unsigned int K = (unsigned) sizeof(X)*8 - 1 - (unsigned) __builtin_clz(X); \
    I =  (unsigned)((K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1)));\
  }\
}

#define COUNT 100000000
int main()
{

    unsigned m = 0;
    srand(1);
    do {
        ll::time_trace t;
        unsigned n = COUNT;
        while (n--) {
            unsigned k = rand();
            if (k > 0) {
                m += __builtin_clz(k);
            }
        }
        cout << ll::time_prec_msec::to_precval(t.check()) << endl;
    } while (0);

    cout << m << endl;
    srand(1);

    do {
        ll::time_trace t;
        unsigned n = COUNT;
        while (n--) {
            unsigned k = rand();
            m += ll::bitorder::order(k);
        }
        cout << ll::time_prec_msec::to_precval(t.check()) << endl;
    } while (0);

#if 0
    foo f;
    int n = 100;

    ll::signal<int(), ll::malloc_allocator> sig;
    sig.connect(f, n);
    auto slot = sig.connect(&foo::dodo, f, 100);
    sig.emit();
    sig.disconnect(slot);
    sig.emit();

    ll::signal<int(), ll::mallocator, true> once_sig;
    once_sig.connect(f, n);
    once_sig.emit();
    once_sig.connect(&foo::dodo, f, 100);
    once_sig.emit();
#endif
    return 0;
}


#if 0
/* assign tree index for size S to variable I. Use x86 asm if possible  */
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define compute_tree_index(S, I)\
{\
  unsigned int X = S >> TREEBIN_SHIFT;\
  if (X == 0)\
    I = 0;\
  else if (X > 0xFFFF)\
    I = NTREEBINS-1;\
  else {\
    unsigned int K = (unsigned) sizeof(X)*__CHAR_BIT__ - 1 - (unsigned) __builtin_clz(X); \
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1)));\
  }\
}

#elif defined (__INTEL_COMPILER)
#define compute_tree_index(S, I)\
{\
  size_t X = S >> TREEBIN_SHIFT;\
  if (X == 0)\
    I = 0;\
  else if (X > 0xFFFF)\
    I = NTREEBINS-1;\
  else {\
    unsigned int K = _bit_scan_reverse (X); \
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1)));\
  }\
}

#elif defined(_MSC_VER) && _MSC_VER>=1300
#define compute_tree_index(S, I)\
{\
  size_t X = S >> TREEBIN_SHIFT;\
  if (X == 0)\
    I = 0;\
  else if (X > 0xFFFF)\
    I = NTREEBINS-1;\
  else {\
    unsigned int K;\
    _BitScanReverse((DWORD *) &K, (DWORD) X);\
    I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1)));\
  }\
}

#else /* GNUC */
#define compute_tree_index(S, I)\
{\
  size_t X = S >> TREEBIN_SHIFT;\
  if (X == 0)\
    I = 0;\
  else if (X > 0xFFFF)\
    I = NTREEBINS-1;\
  else {\
    unsigned int Y = (unsigned int)X;\
    unsigned int N = ((Y - 0x100) >> 16) & 8;\
    unsigned int K = (((Y <<= N) - 0x1000) >> 16) & 4;\
    N += K;\
    N += K = (((Y <<= K) - 0x4000) >> 16) & 2;\
    K = 14 - N + ((Y <<= K) >> 15);\
    I = (K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1));\
  }\
}
#endif /* GNUC */

/* Bit representing maximum resolved size in a treebin at i */
#define bit_for_tree_index(i) \
   (i == NTREEBINS-1)? (SIZE_T_BITSIZE-1) : (((i) >> 1) + TREEBIN_SHIFT - 2)

/* Shift placing maximum resolved bit in a treebin at i as sign bit */
#define leftshift_for_tree_index(i) \
   ((i == NTREEBINS-1)? 0 : \
    ((SIZE_T_BITSIZE-SIZE_T_ONE) - (((i) >> 1) + TREEBIN_SHIFT - 2)))

/* The size of the smallest chunk held in bin with index i */
#define minsize_for_tree_index(i) \
   ((SIZE_T_ONE << (((i) >> 1) + TREEBIN_SHIFT)) |  \
   (((size_t)((i) & SIZE_T_ONE)) << (((i) >> 1) + TREEBIN_SHIFT - 1)))

#endif
