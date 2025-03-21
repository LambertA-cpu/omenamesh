
#ifndef __CJLF_TYPES
#define __CJLF_TYPES
/*we have a global lock manager*/
#define GLOBAL_MAIN __attribute__((constructor))

#define OMENAMESH_API __attribute__((visibility("default")))
#define ARRAY_SIZE(arr) ((sizeof(arr)) / (sizeof(*arr)))

/*mostly local & APIs that might change in the future*/
#define OMENAMESH_API_T __attribute__((visibility("hidden")))
#define VOLATILE OMENAMESH_API_T

typedef unsigned char u8__CJLF;
typedef unsigned short u16__CJLF;
typedef unsigned u32__CJLF;
typedef unsigned long long u64__CJLF;
typedef signed char i8__CJLF;
typedef short i16__CJLF;
typedef int i32__CJLF;
typedef long long i64__CJLF;

typedef void __CJLF_GENERICS;

/*common*/
#include <malloc.h>  /*virtual mem*/
#include <pthread.h> /*posix threads*/
#include <stdbool.h> /*bool*/
#include <stdint.h>  /*types*/
#include <stdio.h>   /*I/O*/
#include <stdlib.h>
#include <string.h> /*string*/
#include <unistd.h>
#endif /*! __CJLF_TYPES*/