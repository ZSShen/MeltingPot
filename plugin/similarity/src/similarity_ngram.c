#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fuzzy.h>
#include "except.h"
#include "spew.h"
#include "similarity.h"
#include "similarity_ngram.h"


void
_SimHashNaive(char *szBin, uint32_t uiLen, uint64_t *p_ulBg)
{
    *p_ulBg = 0;
    uint32_t uiIdx;
    for (uiIdx = 0 ; uiIdx < uiLen ; uiIdx++) {
        char ch = szBin[uiIdx];
        uint64_t ulNg = *p_ulBg + ch;
        *p_ulBg = ulNg & (BLOOM_FILTER_WIDTH - 1);
    }

    return;
}


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

    *p_szHash = (char*)malloc(sizeof(char) * (BLOOM_FILTER_WIDTH + 1));
    if (*p_szHash == NULL)
        EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    char *szHash = *p_szHash;
    memset(szHash, MARKER_MISS, sizeof(char) * BLOOM_FILTER_WIDTH);
    szHash[BLOOM_FILTER_WIDTH] = 0;

    uint32_t uiIdxBgn = 0;
    uint32_t uiIdxEnd = NGRAM_DIMENSION - 1;
    uint64_t ulNg;
    uint8_t ucIdx;
    while (uiIdxEnd < uiLenBuf) {
        _SimHashNaive(szBin + uiIdxBgn, NGRAM_DIMENSION, &ulNg);
        ucIdx = ulNg;
        szHash[ucIdx] = MARKER_HIT;
        uiIdxBgn++;
        uiIdxEnd++;
    }

    if (p_uiLenHash)
        *p_uiLenHash = BLOOM_FILTER_WIDTH + 1;

EXIT:
    return cRtnCode;
}


int8_t
SimCompareHashPair(char *szHashSrc, uint32_t uiLenSrc, 
                   char *szHashTge, uint32_t uiLenTge, uint8_t *p_ucSim)
{
    uint64_t ulInter = 0;
    uint64_t ulUnion = 0;
    uint32_t uiIdx;
    for (uiIdx = 0 ; uiIdx < uiLenSrc ; uiIdx++) {
        if ((szHashSrc[uiIdx] == MARKER_HIT) && (szHashTge[uiIdx] == MARKER_HIT))
            ulInter++;
        if ((szHashSrc[uiIdx] == MARKER_HIT) || (szHashTge[uiIdx] == MARKER_HIT))
            ulUnion++;    
    }
    if (ulUnion)
        *p_ucSim = (ulInter / ulUnion) * 100;
    else
        *p_ucSim = 0;

    return SUCCESS;
}
