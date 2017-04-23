/**
 *   The MIT License (MIT)
 *   Copyright (C) 2014-2017 ZongXian Shen <andy.zsshen@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 */


#ifndef _PATTERN_H_
#define _PATTERN_H_


#include <stdint.h>
#include "data.h"
#include "cluster.h"
#include "slice.h"
#include "similarity.h"


/* The constants helping for common byte block extraction. */
#define ROLL_SHFT_COUNT         (16)    /* The number of shifting bytes. */
#define WILD_CARD_MARK          (0x100) /* The marker for wildcard character. */
#define BYTE_NOISE_00           (0x00)  /* The noisy bytes. */
#define BYTE_NOISE_FF           (0xff)
#define EXTENSION_MASK          (0xff)  /* The mask used to extend char type to short. */
#define THLD_DNMNTR             (100)   /* The denominator for threshold computation. */

/* The constants helping for YARA pattern creation. */
#define HEX_CHUNK_SIZE          (16)    /* The maximum number of bytes in single line. */
#define PREFIX_PATTERN          "AUTO"  /* The prefix for pattern name. */
#define PREFIX_HEX_STRING       "SEQ"   /* The prefix for hex string name. */
#define MODULE_PE               "pe"    /* The external module named "pe". */
#define SPACE_SUBS_TAB          "    "  /* The spaces substituting a tab. */
#define DIGIT_COUNT_ULONG       (20)    /* The maximum number of digits to 
                                           form an unsigned long variable. */
#define BUF_SIZE_PTN_FILE       (4096)  /* The maximum pattern file size. */
#define BUF_SIZE_PTN_SECTION    (1024)  /* The maximum pattern section size. */
#define BUF_SIZE_INDENT         (64)    /* The maxumum indentation length. */


typedef struct THREAD_CRAFT_T {
    int8_t cRtnCode;
    pthread_t tId;
    GROUP *p_Grp;
} THREAD_CRAFT;


typedef struct THREAD_SLOT_T {
    uint16_t usSizeMinSlc;
    uint64_t ulIdxBgn;
    uint64_t ulIdxEnd;
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
PtnOutputResult();


#endif
