
#line 1 "cgo-builtin-prolog"
#include <stddef.h> /* for ptrdiff_t and size_t below */

/* Define intgo when compiling with GCC.  */
typedef ptrdiff_t intgo;

#define GO_CGO_GOSTRING_TYPEDEF
typedef struct { const char *p; intgo n; } _GoString_;
typedef struct { char *p; intgo n; intgo c; } _GoBytes_;
_GoString_ GoString(char *p);
_GoString_ GoStringN(char *p, int l);
_GoBytes_ GoBytes(void *p, int n);
char *CString(_GoString_);
void *CBytes(_GoBytes_);
void *_CMalloc(size_t);

__attribute__ ((unused))
static size_t _GoStringLen(_GoString_ s) { return (size_t)s.n; }

__attribute__ ((unused))
static const char *_GoStringPtr(_GoString_ s) { return s.p; }

#line 3 "/home/karthik/proj/client/client.go"



#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "PDSUs.h"
#include "PDSU.h"
#include "MeasValue.h"
#include "SubCounterIndexType.h"
#include "SubCounterListType.h"
#include <time.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

void decode(int i)
{
    const char *filename = i==1?"decode1":"decode";
    PDSUs_t *pdsud;
    pdsud = calloc(1,sizeof(PDSUs_t));
    char buf[1024000];
    FILE *fp = fopen(filename,"r");
    size_t size = fread(buf,1,sizeof(buf),fp);
    fclose(fp);
    fprintf(stderr, "size - %lu\n" ,size);
    asn_dec_rval_t ret1 = uper_decode(0,&asn_DEF_PDSUs,(void **)&pdsud,buf,size,0,0);
    if(ret1.code==RC_WMORE)
    {
        assert(1==0);
    }else if(ret1.code==RC_FAIL)
    {
        assert(1==2);
    }
    asn_fprint(stderr,&asn_DEF_PDSU,pdsud->list.array[0]);
}

#line 1 "cgo-generated-wrapper"


#line 1 "cgo-gcc-prolog"
/*
  If x and y are not equal, the type will be invalid
  (have a negative array count) and an inscrutable error will come
  out of the compiler and hopefully mention "name".
*/
#define __cgo_compile_assert_eq(x, y, name) typedef char name[(x-y)*(x-y)*-2UL+1UL];

/* Check at compile time that the sizes we use match our expectations. */
#define __cgo_size_assert(t, n) __cgo_compile_assert_eq(sizeof(t), (size_t)n, _cgo_sizeof_##t##_is_not_##n)

__cgo_size_assert(char, 1)
__cgo_size_assert(short, 2)
__cgo_size_assert(int, 4)
typedef long long __cgo_long_long;
__cgo_size_assert(__cgo_long_long, 8)
__cgo_size_assert(float, 4)
__cgo_size_assert(double, 8)

extern char* _cgo_topofstack(void);

/*
  We use packed structs, but they are always aligned.
  The pragmas and address-of-packed-member are only recognized as warning
  groups in clang 4.0+, so ignore unknown pragmas first.
*/
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"

#include <errno.h>
#include <string.h>


#define CGO_NO_SANITIZE_THREAD
#define _cgo_tsan_acquire()
#define _cgo_tsan_release()


#define _cgo_msan_write(addr, sz)

CGO_NO_SANITIZE_THREAD
void
_cgo_c8e6a7261cd2_Cfunc_decode(void *v)
{
	struct {
		int p0;
		char __pad4[4];
	} __attribute__((__packed__, __gcc_struct__)) *_cgo_a = v;
	_cgo_tsan_acquire();
	decode(_cgo_a->p0);
	_cgo_tsan_release();
}

