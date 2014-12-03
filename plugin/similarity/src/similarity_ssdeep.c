#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fuzzy.h>
#include "spew.h"
#include "similarity.h"
#include "similarity_ssdeep.h"


int8_t
SimInit()
{
    return SIM_SUCCESS;
}


int8_t
SimDeinit()
{
    return SIM_SUCCESS;
}


int8_t
SimGetHash(char *szBin, uint32_t uiLenBuf, char **p_szHash, uint32_t *p_uiLenHash)
{
    int8_t cRtnCode = SIM_SUCCESS;

    *p_szHash = (char*)malloc(sizeof(char) * FUZZY_MAX_RESULT);
    if (*p_szHash == NULL) {
        EXIT1(SIM_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));
    }

    /* Apply ssdeep libaray to covert the binary sequence into fuzzy hash string. */
    int32_t iStat = fuzzy_hash_buf(szBin, uiLenBuf, *p_szHash);
    if (iStat != 0) {
        free(p_szHash);
        p_szHash = NULL;
        EXIT1(SIM_FAIL_LIBRARY_CALL, EXIT, "Error: %s.", FAIL_EXTERNAL_LIBRARY_CALL);
    }
    *p_uiLenHash = strlen(*p_szHash);

EXIT:
    return cRtnCode;
}


int8_t
SimCompareHashPair(char *szHashSrc, uint32_t uiLenSrc, 
                   char *szHashTge, uint32_t uiLenTge, uint8_t *p_ucSim)
{
    int8_t cRtnCode = SIM_SUCCESS;

    /* Apply ssdeep libaray to compute the similarity between a pair of ssdeep
       hash strings. */
    int32_t iStat = fuzzy_compare(szHashSrc, szHashTge);
    if (iStat == -1) {
        EXIT1(SIM_FAIL_LIBRARY_CALL, EXIT, "Error: %s.", FAIL_EXTERNAL_LIBRARY_CALL);
    }
    *p_ucSim = iStat;    

EXIT:
    return cRtnCode;
}
