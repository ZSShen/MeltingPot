#ifndef _H_DS_
#define _H_DS_


#include <stdint.h>
#include "utarray.h"


/* Define the buffer size. */
#define BUF_SIZE_SMALL      64
#define BUF_SIZE_MEDIUM     BUF_SIZE_SMALL  << 3
#define BUF_SIZE_LARGE      BUF_SIZE_MEDIUM << 3


/* Redefine the nameing of ut-sereis macros to synchronize coding convention. */
#define ARRAY_ELTPTR(p, q)      utarray_eltptr(p, q)      
#define ARRAY_NEW(p, q)         utarray_new(p, q)
#define ARRAY_FREE(p)           utarray_free(p)
#define ARRAY_LEN(p)            utarray_len(p)
#define ARRAY_NEXT(p, q)        utarray_next(p, q)


/* The ds to record detail information for each section binary. */
typedef struct _BINARY {
    union {
        uint32_t uiIdBin;
        uint32_t uiIdGrp;
    };
    uint16_t usSectIdx;
    uint32_t uiSectOfst;
    uint32_t uiSectSize;
    char *szNameSample;
    char *szHash;
} BINARY;


/* The ds to record the binary pairs with similarity fitting the threshold. */
typedef struct _RELATION {
    uint32_t uiIdBinSrc;
    uint32_t uiIdBinTge;
    struct _RELATION *next;         /* Fit the utlist standard. */
} RELATION;


/* The ds to pass information for thread processing. */
typedef struct _THREAD_PARAM {
    uint32_t uiBinCount;
    uint8_t ucThreadId;
    uint8_t ucThreadCount;
    uint8_t ucSimThrld;
    RELATION *listRelationHead;
    RELATION *listRelationTail;
} THREAD_PARAM;

typedef struct _FAMILY_MEMBER {
    uint32_t uiIdBin;
    struct _FAMILY_MEMBER *prev;    /* Fit the utlist standard. */
    struct _FAMILY_MEMBER *next;    /* Fit the utlist standard. */
} FAMILY_MEMBER;

typedef struct _FAMILY {
    uint32_t uiIdRep;
} FAMILY;


#endif
