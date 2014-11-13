#ifndef _H_DS_
#define _H_DS_


#include <stdint.h>
#include "utarray.h"
#include "utlist.h"
#include "uthash.h"


/*======================================================================*
 *             Declaration for Shared and Custmoized Macros             *
 *======================================================================*/
/* Define the buffer size. */
#define BUF_SIZE_SMALL          64
#define BUF_SIZE_MEDIUM         BUF_SIZE_SMALL  << 3
#define BUF_SIZE_LARGE          BUF_SIZE_MEDIUM << 3


/* Redefine the nameing of ut-sereis macros to synchronize coding convention. */
#define ARRAY_ELTPTR(p, q)      utarray_eltptr(p, q)      
#define ARRAY_NEW(p, q)         utarray_new(p, q)
#define ARRAY_FREE(p)           utarray_free(p)
#define ARRAY_LEN(p)            utarray_len(p)
#define ARRAY_NEXT(p, q)        utarray_next(p, q)

#define DL_FREE(p, q)           DL_DELETE(p, q);                       \
                                free(q);

#define HASH_FREE(p, q, r)      HASH_DELETE(p, q, r);                  \
                                free(r);

/*======================================================================*
 *                Declaration for Common Data Structures                *
 *======================================================================*/
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
    struct _RELATION *prev;             /* Fit the utlist standard. */
    struct _RELATION *next;             /* Fit the utlist standard. */
} RELATION;


/* The ds to pass information for thread processing. */
typedef struct THREAD_PARAM {
    uint32_t uiBinCount;
    uint8_t ucThreadId;
    uint8_t ucThreadCount;
    uint8_t ucSimThrld;
    RELATION *pRelHead;
} THREAD_PARAM;


/* The ds to record member binary id of a family. */
typedef struct _FAMILY_MEMBER {
    uint32_t uiIdBin;
    struct _FAMILY_MEMBER *prev;        /* Fit the utlist standard. */
    struct _FAMILY_MEMBER *next;        /* Fit the utlist standard. */
} FAMILY_MEMBER;


/* The ds to record the members of a family. */
typedef struct _FAMILY {
    uint32_t uiIdRep;
    FAMILY_MEMBER *pFamMbrHead;
    UT_hash_handle hh;                  /* Fit the uthash standard. */
} FAMILY;


/* The bridge interface for other modules to get the clustering result. */
typedef struct _GROUP_RESULT {
    UT_array *pAResBin;
    FAMILY *pMapResFamily;
} GROUP_RESULT;


/*======================================================================*
 *                  Declaration for Customized Handlers                 *
 *======================================================================*/
/**
 * The utility to guide utarray for BINARY structure copy.
 *
 * @param vpTge     The pointer to the target.
 * @param vpSrc     The pointer to the source object.
 */
void UTArrayBinaryCopy(void *vpTge, const void *vpSrc);

/**
 * The utility to guide utarray for BINARY structure release.
 *
 * @param vpCurr     The pointer to the to be released object.
 */
void UTArrayBinaryDeinit(void *vpCurr);


#endif
