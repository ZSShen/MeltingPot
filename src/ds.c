#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ds.h"


/*======================================================================*
 *               Implementation for Customized Functions                *
 *======================================================================*/
/**
 * !EXTERNAL
 * _UTArrayBinaryCopy(): Guide utarray for BINARY structure copy.
 */
void UTArrayBinaryCopy(void *vpTge, const void *vpSrc) {
    BINARY *pBinSrc, *pBinTge;
    
    pBinSrc = (BINARY*)vpSrc;
    pBinTge = (BINARY*)vpTge;
    
    pBinTge->uiIdBin = pBinSrc->uiIdBin;
    pBinTge->uiIdGrp = pBinSrc->uiIdGrp;
    pBinTge->usSectIdx = pBinSrc->usSectIdx;
    pBinTge->uiSectOfst = pBinSrc->uiSectOfst;
    pBinTge->uiSectSize = pBinSrc->uiSectSize;
    
    /* Duplicate the sample name. */
    if (pBinSrc->szNameSample != NULL) {
        pBinTge->szNameSample = strdup(pBinSrc->szNameSample);
        assert(pBinTge->szNameSample != NULL);
        free(pBinSrc->szNameSample);
    } else {
        pBinTge->szNameSample = NULL;
    }
    /* Duplicate the section szHash. */
    if (pBinSrc->szHash != NULL) {
        pBinTge->szHash = strdup(pBinSrc->szHash);
        assert(pBinTge->szHash != NULL);
        free(pBinSrc->szHash);
    } else {
        pBinTge->szHash = NULL;
    }

    return;
}

/**
 * !EXTERNAL
 * _UTArrayBinaryDeinit(): Guide utarray for BINARY structure release.
 */
void UTArrayBinaryDeinit(void *vpCurr) {
    BINARY *pBin;
    
    pBin = (BINARY*)vpCurr;
    if (pBin->szNameSample != NULL) {
        free(pBin->szNameSample);
    }
    if (pBin->szHash != NULL) {
        free(pBin->szHash);
    }

    return;
}
