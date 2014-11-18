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
    pSeqTge->ucDontCareCnt = pSeqSrc->ucDontCareCnt;
    pSeqTge->ucByteCnt = pSeqSrc->ucByteCnt;
    pSeqTge->aByte = NULL;
    pSeqTge->pSetSectIdx = NULL;
    
    /* Duplicate the byte sequence. */
    if (pSeqSrc->aByte != NULL) {
        pSeqTge->aByte = (uint16_t*)malloc(sizeof(uint16_t) * pSeqTge->ucByteCnt);
        assert(pSeqTge->aByte != NULL);
        memcpy(pSeqTge->aByte, pSeqSrc->aByte, sizeof(uint16_t) * pSeqTge->ucByteCnt);
        free(pSeqSrc->aByte);
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
    if (pSeq->aByte != NULL) {
        free(pSeq->aByte);
    }
	
	if (pSeq->pSetSectIdx != NULL) {
		HASH_ITER(hh, pSeq->pSetSectIdx, pSetCurr, pSetHelp) {
			HASH_FREE(hh, pSeq->pSetSectIdx, pSetCurr);
		}
	}

    return;
}

/**
 * !EXTERNAL
 * UTArraySequenceSort(): Guide utarray for SEQUENCE structure sorting.
 */
int UTArraySequenceSort(const void *vpSrc, const void *vpTge) {
    SEQUENCE *pSeqSrc, *pSeqTge;

    pSeqSrc = (SEQUENCE*)vpSrc;
    pSeqTge = (SEQUENCE*)vpTge;

    return pSeqSrc->ucDontCareCnt - pSeqTge->ucDontCareCnt;
}


int UTHashFamilySort(const void *vpSrc, const void *vpTge) {
    int iSizeSrc, iSizeTge;
    FAMILY *pFamSrc, *pFamTge;

    pFamSrc = (FAMILY*)vpSrc;
    pFamTge = (FAMILY*)vpTge;
    iSizeSrc = ARRAY_LEN(pFamSrc->aFamMbr);
    iSizeTge = ARRAY_LEN(pFamTge->aFamMbr);

    return iSizeSrc - iSizeTge;
}
