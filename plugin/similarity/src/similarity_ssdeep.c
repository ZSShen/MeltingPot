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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fuzzy.h>
#include "except.h"
#include "spew.h"
#include "similarity.h"
#include "similarity_ssdeep.h"


int8_t
SimInit()
{
    return SUCCESS;
}


int8_t
SimDeinit()
{
    return SUCCESS;
}


int8_t
SimGetHash(char *szBin, uint32_t uiLenBuf, char **p_szHash, uint32_t *p_uiLenHash)
{
    int8_t cRtnCode = SUCCESS;

    *p_szHash = (char*)malloc(sizeof(char) * FUZZY_MAX_RESULT);
    if (*p_szHash == NULL)
        EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    /* Apply ssdeep libaray to covert the binary sequence into fuzzy hash string. */
    int32_t iStat = fuzzy_hash_buf(szBin, uiLenBuf, *p_szHash);
    if (iStat != 0) {
        free(p_szHash);
        p_szHash = NULL;
        EXIT1(FAIL_EXT_LIB_CALL, EXIT, "Error: %s.", FAIL_EXTERNAL_LIBRARY_CALL);
    }

    if (p_uiLenHash)
        *p_uiLenHash = strlen(*p_szHash);

EXIT:
    return cRtnCode;
}


int8_t
SimCompareHashPair(char *szHashSrc, uint32_t uiLenSrc, 
                   char *szHashTge, uint32_t uiLenTge, uint8_t *p_ucSim)
{
    int8_t cRtnCode = SUCCESS;

    /* Apply ssdeep libaray to compute the similarity between a pair of ssdeep
       hash strings. */
    int32_t iStat = fuzzy_compare(szHashSrc, szHashTge);
    if (iStat == -1)
        EXIT1(FAIL_EXT_LIB_CALL, EXIT, "Error: %s.", FAIL_EXTERNAL_LIBRARY_CALL);

    *p_ucSim = iStat;    

EXIT:
    return cRtnCode;
}
