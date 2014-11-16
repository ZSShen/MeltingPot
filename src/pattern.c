#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <semaphore.h>
#include "ds.h"
#include "spew.h"
#include "pattern.h"


#define DONT_CARE_MARK          0x100


/*======================================================================*
 *                   Declaration for Private Datatype                   *
 *======================================================================*/
/* The ds to pass information for thread processing. */
typedef struct THREAD_PARAM {
    uint32_t uiThreadId;
    FAMILY *pFam;
} THREAD_PARAM;


/*======================================================================*
 *                    Declaration for Private Object                    *
 *======================================================================*/
static sem_t _pSem;
static CONFIG *_pCfg;
static UT_array *_aBin;


/*======================================================================*
 *                  Declaration for Internal Functions                  *
 *======================================================================*/
static void* _PtnGrepByteSequenceFromCluster(void *vpThreadParam);

static int _PtnGrepByteSequencePerSlot(UT_array *aFamMbr, uint32_t uiBlkCount,
                                       uint32_t uiIdxSlot, UT_array *aSeq);

/*======================================================================*
 *                Implementation for External Functions                 *
 *======================================================================*/
/**
 * !EXTERNAL
 * PtnInitTask(): The constructor of PATTERN structure.
 */
int PtnInitTask(PATTERN *self, CONFIG *pCfg) {
    
    _pCfg = pCfg;
    self->locateByteSequence = PtnLocateByteSequence;
    self->generatePattern = PtnGeneratePattern;    

    return 0;
}

/**
 * !EXTERNAL
 * PtnDeinitTask(): The destructor of PATTERN structure.
 */
int PtnDeinitTask(PATTERN *self) {

    return 0;
}

/**
 * !EXTERNAL
 * PtnLocateBinarySequence(): Select a set of candidates which represent the 
 * similar binary sequence of the clustered PE sections.
 */
int PtnLocateByteSequence(PATTERN *self, GROUP_RESULT *pGrpRes) {
    int iRtnCode;
    uint8_t ucParallelity, ucBlkCount, ucBlkSize;
    uint32_t uiIter, uiBinCount, uiGrpCount, uiSlotCount;
    UT_array *pABin;
    FAMILY *pMapFam;
    pthread_t *aThread;
    FAMILY *pFamCurr, *pFamHelp;
    THREAD_PARAM *aThreadParam;
    
    iRtnCode = 0;
    _aBin = pGrpRes->pABin;
    /* Prepare the thread parameters. */
    uiGrpCount = HASH_CNT(hh, pGrpRes->pMapFam);
    aThread = NULL;
    aThreadParam = NULL;
    aThread = (pthread_t*)malloc(sizeof(pthread_t) * uiGrpCount);
    if (aThread == NULL) {
        Spew0("Error: Cannot allocate the array for thread id.");
        iRtnCode = -1;
        goto EXIT;    
    }
    aThreadParam = (THREAD_PARAM*)malloc(sizeof(THREAD_PARAM) * uiGrpCount);
    if (aThreadParam == NULL) {
        Spew0("Error: Cannot allocate the array for thread parameters.");
        iRtnCode = -1;
        goto EXIT;
    }
    
    /* Prepare the thread semaphore. */
    sem_init(&_pSem, 0, uiGrpCount);
    
	/* Fork a dedicated thread to search the similar binary sequences for one group. 
	   But the thread replication is controlled by the semphore with the quantity 
	   limited by the parallelity configuration. */
    uiIter = 0;
    HASH_ITER (hh, pGrpRes->pMapFam, pFamCurr, pFamHelp) {
        sem_wait(&_pSem);
        aThreadParam[uiIter].uiThreadId = uiIter + 1;
        aThreadParam[uiIter].pFam = pFamCurr;
        pthread_create(&aThread[uiIter], NULL, _PtnGrepByteSequenceFromCluster, 
                       (void*)&(aThreadParam[uiIter]));
        uiIter++;
    }
    for (uiIter = 0 ; uiIter < uiGrpCount ; uiIter++) {
        pthread_join(aThread[uiIter], NULL);
    }
    
    /* Release the thread semaphore. */
    sem_destroy(&_pSem);
    
EXIT:
    if (aThread != NULL) {
        free(aThread);
    } 
    if (aThreadParam != NULL) {
        free(aThreadParam);
    }
    
    return iRtnCode;
}

/**
 * !EXTERNAL
 * PtnGeneratePattern(): Output the set of candidates each of which is outputted 
 * as a Yara-formatted pattern.
 */
int PtnGeneratePattern(PATTERN *self) {

    return 0;
}


/*======================================================================*
 *                Implementation for Internal Functions                 *
 *======================================================================*/
static void* _PtnGrepByteSequenceFromCluster(void *vpThreadParam) {
    uint32_t uiIter, uiBinCount, uiSlotCount;
    THREAD_PARAM *pThreadParam;
    UT_array *aSeq, *aFamMbr;
    UT_array **aASeq;
    
    pThreadParam = (THREAD_PARAM*)vpThreadParam;
    aFamMbr = pThreadParam->pFam->aFamMbr;
    uiBinCount = ARRAY_LEN(aFamMbr);	
    uiSlotCount = (uint32_t)ceil((double)uiBinCount / _pCfg->ucIoBandwidth);
    aASeq = NULL;
    aASeq = (UT_array**)malloc(sizeof(UT_array*) * uiSlotCount);
    if (aASeq == NULL) {
        Spew0("Error: Cannot allocate the array of SEQUENCEs for job slots.");
        goto EXIT;   
    }    

    /* Evenly divide the clustered sections into slots. For each slot, an array of 
       SEQUENCE structures is preared to record the similar byte sequences shared by 
       the sections belonged to that slot. */
    UT_icd icdSeq = { sizeof(SEQUENCE), NULL, UTArraySequenceCopy, UTArraySequenceDeinit};
    for (uiIter = 0 ; uiIter < uiSlotCount ; uiIter++) {
        aASeq[uiIter] = NULL;
        ARRAY_NEW(aASeq[uiIter], &icdSeq);
        _PtnGrepByteSequencePerSlot(aFamMbr, uiBinCount, uiIter, aASeq[uiIter]);
    }

EXIT:
    if (aASeq != NULL) {
        for (uiIter = 0 ; uiIter < uiSlotCount ; uiIter++) {
            if (aASeq[uiIter] != NULL) {
                ARRAY_FREE(aASeq[uiIter]);
            }
        }
        free(aASeq);        
    }
    /* Resume the thread creater. */
    sem_post(&_pSem);
    printf("=== Thread %3d === Finish the job.\n", pThreadParam->uiThreadId);

    return;
}

static int _PtnGrepByteSequencePerSlot(UT_array *aFamMbr, uint32_t uiMaxBinCount,
                                       uint32_t uiIdxSlot, UT_array *aSeq) {
	int iRtnCode, iStatus;
    uint8_t ucIterCmp, ucBlkSize, ucDcCount;
    uint32_t uiIterBin, uiIterRnd, uiIdxBin, uiIdxBgn, uiIdxEnd;
    uint32_t uiOfst, uiBinCount, uiMinBinSize, uiRound;
    uint16_t *bufCmp, *aSectIdx;
    uint32_t *pUiIdxBin;
    size_t nExptRead, nRealRead;
    bool bIoExcept;
    FILE *fpSample;
    BINARY *pBin;
    SEQUENCE *pSeq;
    SECTION_SET *pSetCurr;
    char **aBinContent;
    char szPathSample[BUF_SIZE_MEDIUM];

	iRtnCode = 0;
    uiIdxBgn = uiIdxSlot * _pCfg->ucIoBandwidth;
    uiIdxEnd = MIN(uiIdxBgn + _pCfg->ucIoBandwidth, uiMaxBinCount);
    uiBinCount = uiIdxEnd - uiIdxBgn;
    ucBlkSize = _pCfg->ucBlkSize;
    bufCmp = NULL;
    aBinContent = NULL;
    aSectIdx = NULL;
    
    /* Preparation Works:
       1. Determine the number of rounds to search and compare byte sequences. 
       2. Prepare the buffer for byte sequences comparison. 
       3. Load the contents of sections belonged to the current slot. */
    bufCmp = (uint16_t*)malloc(sizeof(uint16_t*) * ucBlkSize);
    if (bufCmp == NULL) {
        Spew0("Error: Cannot allocate the buffer for byte sequences comparison.");
        iRtnCode = -1;
        goto EXIT;
    }
    aBinContent = (char**)malloc(sizeof(char*) * uiBinCount);
    if (aBinContent == NULL) {
        Spew0("Error: Cannot allocate the array for section contents.");
        iRtnCode = -1;
        goto EXIT;
    }
    aSectIdx = (uint16_t*)malloc(sizeof(uint16_t) * uiBinCount);
    if (aSectIdx == NULL) {
        Spew0("Error: Cannot allocate the array for section sizes.");
        iRtnCode = -1;
        goto EXIT;
    }
    for (uiIterBin = 0 ; uiIterBin < uiBinCount ; uiIterBin++) {
        aBinContent[uiIterBin] = NULL;
    }
    
    uiMinBinSize = UINT_MAX;
    for (uiIterBin = uiIdxBgn ; uiIterBin < uiIdxEnd ; uiIterBin++) {
        pUiIdxBin = (uint32_t*)ARRAY_ELTPTR(aFamMbr, uiIterBin);
        pBin = (BINARY*)ARRAY_ELTPTR(_aBin, *pUiIdxBin);
        
        memset(szPathSample, 0, sizeof(char) * BUF_SIZE_MEDIUM);    
        snprintf(szPathSample, BUF_SIZE_MEDIUM, "%s\%s", _pCfg->szPathInput, 
                 pBin->szNameSample);    
        fpSample = fopen(szPathSample, "rb");
        if (fpSample == NULL) {
            Spew1("Error: %s.", strerror(errno));
            iRtnCode = -1;
            goto EXIT;
        }
        
        nExptRead = pBin->uiSectSize;
        uiIdxBin = uiIterBin - uiIdxBgn;
        aBinContent[uiIdxBin] = (char*)malloc(sizeof(char) * nExptRead);
        if (aBinContent[uiIdxBin] == NULL) {
            Spew0("Error: Cannot allocate the buffer for single section.");
            iRtnCode = -1;
            goto EXIT;
        }
        aSectIdx[uiIdxBin] = pBin->usSectIdx;
        bIoExcept= false;
        iStatus = fseek(fpSample, pBin->uiSectOfst, SEEK_SET);
        if (iStatus != 0) {
            Spew1("Error: %s.", strerror(errno));
            bIoExcept = true;
            goto CLOSE;
        }
        nRealRead = fread(aBinContent[uiIdxBin], sizeof(char), nExptRead, fpSample);
        if (nRealRead != nExptRead) {
            Spew1("Error: %s.", strerror(errno));
            bIoExcept = true;
            goto CLOSE;
        }
        
    CLOSE:
        fclose(fpSample);
        if (bIoExcept == true) {
            iRtnCode = -1;
            goto EXIT;
        }
        if (nExptRead < uiMinBinSize) {
            uiMinBinSize = nExptRead;
        }
    }
    uiRound = (uint32_t)floor((double)uiMinBinSize / ucBlkSize);

    /* Iteratively scan the byte sequences of all the sections starting from 
       the indicated offset. */
    for (uiIterRnd = 0 ; uiIterRnd < uiRound ; uiIterRnd++) {
        
        /* Use the first section as the source of comparison. */
        uiOfst = uiIterRnd * ucBlkSize;
        for (ucIterCmp = 0 ; ucIterCmp < ucBlkSize ; ucIterCmp++) {
            bufCmp[ucIterCmp] = aBinContent[0][uiOfst + ucIterCmp];    
        }
        ucDcCount = 0;
        
        /* Compare the rest of the sections with the source comparator. */
        for (uiIterBin = 1 ; uiIterBin < uiBinCount ; uiIterBin++) {
            for (ucIterCmp = 0 ; ucIterCmp < ucBlkSize ; ucIterCmp++) {
                if (bufCmp[ucIterCmp] != aBinContent[uiIterBin][uiOfst + ucIterCmp]) {
                    bufCmp[ucIterCmp] = DONT_CARE_MARK;
                    ucDcCount++;
                }
            }
        }
        
        pSeq = (SEQUENCE*)malloc(sizeof(SEQUENCE));
        if (pSeq == NULL) {
            Spew0("Cannot allocate SEQUENCE for byte sequence recording.");
            iRtnCode = -1;
            goto EXIT;
        }
        pSeq->ucDontCareCount = ucDcCount;
        pSeq->ucPayloadSize = ucBlkSize;
        pSeq->uiOfst = uiOfst;
        
        /* Record the indices of sections contributing to the extracted byte sequence.*/
        pSeq->pSetSectIdx = NULL;
        for (uiIterBin = 0 ; uiIterBin < uiBinCount ; uiIterBin++) {
            HASH_FIND(hh, pSeq->pSetSectIdx, &aSectIdx[uiIterBin], sizeof(uint16_t), pSetCurr);
            if (pSetCurr == NULL) {
                pSetCurr = (SECTION_SET*)malloc(sizeof(SECTION_SET));
                if (pSetCurr == NULL) {
                    Spew0("Cannot allocate SECTION_SET for section index recording.");
                    iRtnCode = -1;
                    goto EXIT;
                }
                pSetCurr->usSectIdx = aSectIdx[uiIterBin];
                HASH_ADD(hh, pSeq->pSetSectIdx, usSectIdx, sizeof(uint16_t), pSetCurr);
            }       
        }
        
        /* Record the byte sequence. */
        pSeq->aPayload = (uint16_t*)malloc(sizeof(uint16_t*) * ucBlkSize);
        if (pSeq->aPayload == NULL) {
            Spew0("Cannot allocate buffer for extracted byte sequence.");
            iRtnCode = -1;
            goto EXIT;
        }
        for (ucIterCmp = 0 ; ucIterCmp < ucBlkSize ; ucIterCmp++) {
            pSeq->aPayload[ucIterCmp] = bufCmp[ucIterCmp];
        }
        
        ARRAY_PUSH_BACK(aSeq, pSeq);
    }

EXIT:
    if (bufCmp != NULL) {
        free(bufCmp);
    }
    if (aBinContent != NULL) {
        for (uiIterBin = 0 ; uiIterBin < uiBinCount ; uiIterBin++) {
            if (aBinContent[uiIterBin] != NULL) {
                free(aBinContent[uiIterBin]);
            }
        }
        free(aBinContent);
    }
    if (aSectIdx != NULL) {
        free(aSectIdx);
    }

	return iRtnCode;
}
