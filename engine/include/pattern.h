#ifndef _PATTERN_H_
#define _PATTERN_H_


#include <stdint.h>
#include "data.h"
#include "cluster.h"
#include "slice.h"
#include "similarity.h"


/* The number of shifting bytes applied for common block extraction. */
#define ROLLING_SHIFT_SIZE      16


typedef struct THREAD_CRAFT_T {
    int8_t cRtnCode;
    pthread_t tId;
    GROUP *p_Grp;
    GPtrArray *a_BlkCand;
} THREAD_CRAFT;


typedef struct THREAD_SLOT_T {
    uint64_t ulIdxBgn, ulIdxEnd;
    char **a_szBin;
    GArray *a_Mbr;
    GPtrArray *a_BlkCand;
} THREAD_SLOT;


/**
 * This function sets the context which:
 *     1. Provides the user specified configuration.
 *     2. Should be updated with the formal patterns.
 *
 * @param p_Ctx     The pointer to the CONTEXT structure.
 * 
 * @return (currently unused)
 */
int8_t
PtnSetContext(CONTEXT *p_Ctx);


/**
 * For each group, this function extracts a set of byte sequences shared by 
 * the correlated file slices with user designated quality.
 * 
 * @return status code.
 */
int8_t
PtnCraftPattern();


/**
 * This function outputs the byte sequences as YARA format patterns. 
 * 
 * @return status code.
 */
int8_t
PtnOutputYara();


#endif
