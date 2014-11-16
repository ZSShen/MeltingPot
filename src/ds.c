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
    pBinTge->szNameSample = NULL;
    pBinTge->szHash = NULL;
    
    /* Duplicate the sample name. */
    if (pBinSrc->szNameSample != NULL) {
        pBinTge->szNameSample = strdup(pBinSrc->szNameSample);
        assert(pBinTge->szNameSample != NULL);
        free(pBinSrc->szNameSample);
    }
    
    /* Duplicate the section szHash. */
    if (pBinSrc->szHash != NULL) {
        pBinTge->szHash = strdup(pBinSrc->szHash);
        assert(pBinTge->szHash != NULL);
        free(pBinSrc->szHash);
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

/**
 * !EXTERNAL
 * _UTArraySequenceCopy(): Guide utarray for SEQUENCE structure copy.
 */
void UTArraySequenceCopy(void *vpTge, const void *vpSrc) {
    SEQUENCE *pSeqSrc, *pSeqTge;    
	SECTION_SET *pSetSrc, *pSetTge, *pSetHelp;

    pSeqSrc = (SEQUENCE*)vpSrc;
    pSeqTge = (SEQUENCE*)vpTge;
    pSeqTge->uiOfst = pSeqSrc->uiOfst;
    pSeqTge->ucDontCareCount = pSeqSrc->ucDontCareCount;
    pSeqTge->ucPayloadSize = pSeqSrc->ucPayloadSize;
    pSeqTge->aPayload = NULL;
    pSeqTge->pSetSectIdx = NULL;
    
    /* Duplicate the payload. */
    if (pSeqSrc->aPayload != NULL) {
        pSeqTge->aPayload = (uint16_t*)malloc(sizeof(uint16_t) * pSeqTge->ucPayloadSize);
        assert(pSeqTge->aPayload != NULL);
        memcpy(pSeqTge->aPayload, pSeqSrc->aPayload, sizeof(uint16_t) * pSeqTge->ucPayloadSize);
        free(pSeqSrc->aPayload);
    }
	
	/* Duplicate the hash set of section index. */
	if (pSeqSrc->pSetSectIdx != NULL) {
		HASH_ITER(hh, pSeqSrc->pSetSectIdx, pSetSrc, pSetHelp) {
			pSetTge = (SECTION_SET*)malloc(sizeof(SECTION_SET));
			assert(pSetTge != NULL);
			memcpy(pSetTge, pSetSrc, sizeof(SECTION_SET));
			HASH_ADD(hh, pSeqTge->pSetSectIdx, usSectIdx, sizeof(uint16_t), pSetTge);
			HASH_FREE(hh, pSeqSrc->pSetSectIdx, pSetSrc);
		}
	}

    return;
}

/**
 * !EXTERNAL
 * _UTArraySequenceDeinit(): Guide utarray for SEQUENCE structure release.
 */
void UTArraySequenceDeinit(void *vpCurr) {
    SEQUENCE *pSeq;
    SECTION_SET *pSetCurr, *pSetHelp;
    
    pSeq = (SEQUENCE*)vpCurr;
    if (pSeq->aPayload != NULL) {
        free(pSeq->aPayload);
    }
	
	if (pSeq->pSetSectIdx != NULL) {
		HASH_ITER(hh, pSeq->pSetSectIdx, pSetCurr, pSetHelp) {
			HASH_FREE(hh, pSeq->pSetSectIdx, pSetCurr);
		}
	}

    return;
}

