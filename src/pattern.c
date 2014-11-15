#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <semaphore.h>
#include "ds.h"
#include "spew.h"
#include "pattern.h"


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
static uint8_t _ucIoBandwidth;
static uint8_t _ucBlkSize;
static uint8_t _ucBlkCount;
static sem_t _pSem;
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
    
    self->pCfg = pCfg;
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
    _ucIoBandwidth = self->pCfg->ucIoBandwidth;
    _aBin = pGrpRes->pABin;
    /* Prepare the thread parameters. */
    uiGrpCount = HASH_CNT(hh, pGrpRes->pMapFam);
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
        goto FREE_THREAD;
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
    
FREE_PARAM:
    free(aThreadParam);
FREE_THREAD:
    free(aThread);
EXIT:
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
    uiSlotCount = (uint32_t)ceil((double)uiBinCount / _ucIoBandwidth);
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
        _PtnGrepByteSequencePerSlot(aFamMbr, uiBinCount, uiIter, aSeq);
    }
    
    
FREE_SLOT:
    for (uiIter = 0 ; uiIter < uiSlotCount ; uiIter++) {
        if (aASeq[uiIter] != NULL) {
            ARRAY_FREE(aASeq[uiIter]);
        }
    }
    free(aASeq);
EXIT:    
    /* Resume the thread creater. */
    sem_post(&_pSem);

    return;
}

static int _PtnGrepByteSequencePerSlot(UT_array *aFamMbr, uint32_t uiBinCount,
                                       uint32_t uiIdxSlot, UT_array *aSeq) {
	int iRtnCode;
    uint32_t uiIter, uiIdxBgn, uiIdxEnd, uiMinBinSize, uiRound;
    uint32_t *pUiIdxBin;
    BINARY *pBin;

	iRtnCode = 0;
    uiIdxBgn = uiIdxSlot * _ucIoBandwidth;
    uiIdxEnd = MIN(uiIdxBgn + _ucIoBandwidth, uiBinCount);
    
    /* Determine the number of rounds to search and compare byte sequences. */
    uiMinBinSize = UINT_MAX;
    for (uiIter = uiIdxBgn ; uiIter < uiIdxEnd ; uiIter++) {
        pUiIdxBin = (uint32_t*)ARRAY_ELTPTR(aFamMbr, uiIter);
        pBin = (BINARY*)ARRAY_ELTPTR(_aBin, *pUiIdxBin);
        if (pBin->uiSectSize < uiMinBinSize) {
            uiMinBinSize = pBin->uiSectSize;
        }
    }

	return iRtnCode;
}
