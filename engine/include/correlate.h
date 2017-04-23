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


#ifndef _CORRELATE_H_
#define _CORRELATE_H_


#include <stdint.h>
#include "data.h"
#include "cluster.h"
#include "slice.h"
#include "similarity.h"


/* The thread parameter to record the result of file slicing. */
typedef struct THREAD_SLICE_T {
    int8_t cRtnCode;
    pthread_t tId;
    char *szPath;
    GPtrArray *a_Hash;
    GPtrArray *a_Slc;
} THREAD_SLICE;


/* The thread parameter to record the range information for parallel pairwise
   similarity computation. */
typedef struct THREAD_COMPARE_T {
    int8_t cRtnCode;
    uint8_t ucIdThrd;
    pthread_t tId;
    GPtrArray *a_Bind;
} THREAD_COMPARE;


/**
 * This function sets the context which:
 *     1. Provides the user specified configuration and plugins.
 *     2. Will be updated with the correlation result.
 *
 * @param p_Ctx     The pointer to the CONTEXT structure.
 * 
 * @return (currently unused)
 */
int8_t
CrlSetContext(CONTEXT *p_Ctx);


/**
 * This function processes the given set of files, each of which:
 *     1. Will be dissected into slices.
 *     2. Each slice will be hashed into a string for similarity comparison.
 * 
 * @return status code
 */
int8_t
CrlPrepareSlice();


/**
 * This function correlates the similar slices into groups.
 *
 * @return status code
 */
int8_t
CrlCorrelateSlice();


#endif
