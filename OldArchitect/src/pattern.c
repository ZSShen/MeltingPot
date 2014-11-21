#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ds.h"
#include "spew.h"
#include "pattern.h"


/*======================================================================*
 *             Declaration for Pattern Related Information              *
 *======================================================================*/
/* The constants helping for byte sequence recording. */
#define DONT_CARE_MARK          (0x100)     /* The marker for don't care byte. */           
#define EXTENSION_MASK          (0xff)      /* The mask used to extend char to short. */

/* The constants helping for YARA-formatted pattern. */
#define HEX_CHUNK_SIZE          (16)        /* The maximum number of bytes in single line. */
#define PREFIX_PATTERN          "AUTO"      /* The prefix for pattern name. */
#define PREFIX_HEX_STRING       "SEQ"       /* The prefix for hex string name. */
#define MODULE_PE               "pe"        /* The external module named "pe". */
#define SPACE_SUBS_TAB          "    "      /* The spaces substituting a tab. */


/*======================================================================*
 *                   Declaration for Private Datatype                   *
 *======================================================================*/
/* The ds to pass information for thread processing. */
typedef struct THREAD_PARAM {
    uint32_t uiThrdId;
    FAMILY *pFam;
} THREAD_PARAM;


/*======================================================================*
 *                    Declaration for Private Object                    *
 *======================================================================*/
static uint32_t _uiGrpCnt;
static sem_t _pSem;
static CONFIG *_pCfg;
static UT_array *_aBin;
static UT_array **_aRawPtn;


/*======================================================================*
 *                  Declaration for Internal Functions                  *
 *======================================================================*/
 /**
  * This function extracts the similar byte sequences shared by the given 
  * group of sections.
  * 
  * @param vpThrdParam      The pointer to the thread parameter.
  * 
  * @return                 Should be the thread status but currently ignored.
  */
static void* PtnGrepByteSequenceFromGroup(void *vpThrdParam);

/**
 * To avoid memory exhaustion, we split the group into several slots. This
 * function extracts the byte sequences for the given slot and deliver the
 * result to PtnGrepByteSequenceFromGroup() for merging.
 * 
 * @param aFamMbr           The array storing the member ids.
 *                          (The id is used to access BINARY array.)
 * @param uiGrpSize         The size of the given group.
 * @param uiIdxSlot         The index of the given slot.
 * @param aSeq              The array storing byte sequences.
 */
static int _PtnGrepByteSequencePerSlot(UT_array *aFamMbr, uint32_t uiGrpSize,
                                       uint32_t uiIdxSlot, UT_array *aSeq);

/**
 * This function formats the hex byte sequences to be showed in the "string"
 * section of YARA pattern.
 * 
 * @param bufBgn            The front-end pivot to the buffer storing formatted data.
 * @param pUiWrtCnt         The pointer to the value which will be stored with the
 *                          number of bytes formatted in the current round.
 * @param ucIdxBlk          The index to the set of to be outputted byte sequences.
 * @param aByte             The to be outputted byte sequence.  
 * @param ucByteCnt         The size of the sequence.
 */
static void _PtnFormatSectionString(char *bufBgn, uint32_t *pUiWrtCnt,
                                    uint8_t ucIdxBlk, uint16_t *aByte, 
                                    uint8_t ucByteCnt);

/**
 * This function formats the matching offset offsets to be showed in the
 * "conditions" section of YARA pattern.
 * 
 * @param bufBgn            The front-end pivot to the buffer storing formatted data.
 * @param pUiWrtCnt         The pointer to the value which will be stored with the
 *                          number of bytes formatted in the current round.
 * @param ucIdxBlk          The index to the set of to be outputted byte sequences.
 * @param ucBlkCnt          The size of the set.
 * @param uiOfst            The offset to a certain section which will be used
 *                          as the matching condition.
 * @param pSetSectIdx       The set of section ids which are also used as matching
 *                          condition.
 */
static void _PtnFormatSectionCondition(char *bufBgn, uint32_t *pUiWrtCnt,
                                       uint8_t ucIdxBlk, uint8_t ucBlkCnt,
                                       uint32_t uiOfst, SECTION_SET *pSetSectIdx);


/*======================================================================*
 *                Implementation for External Functions                 *
 *======================================================================*/
/**
 * !EXTERNAL
 * PtnInitTask(): The constructor of PATTERN structure.
 */
int PtnInitTask(PATTERN *self, CONFIG *pCfg) {
    
    _pCfg = pCfg;
    _aBin = NULL;
    _aRawPtn = NULL;
    self->extractByteSequence = PtnExtractByteSequence;
    self->generatePattern = PtnGeneratePattern;    

    return 0;
}

/**
 * !EXTERNAL
 * PtnDeinitTask(): The destructor of PATTERN structure.
 */
int PtnDeinitTask(PATTERN *self) {
    uint32_t uiIter;

    if (_aRawPtn != NULL) {    
        for (uiIter = 0 ; uiIter < _uiGrpCnt ; uiIter++) {
            if (_aRawPtn[uiIter] != NULL) {
                ARRAY_FREE(_aRawPtn[uiIter]);            
            }
        }
        free(_aRawPtn);
    }

    return 0;
}

/**
 * !EXTERNAL
 * PtnExtractByteSequence(): Select a set of candidates which represent the 
 * similar byte sequence of the clustered PE sections.
 */
int PtnExtractByteSequence(PATTERN *self, GROUP_RESULT *pGrpRes) {
    int iRtnCode, iStatus;
    uint8_t ucParallelity, ucBlkCnt, ucBlkSize;
    uint32_t uiIter, uiSlotCnt;
    UT_array *pABin;
    FAMILY *pMapFam;
    pthread_t *aThrd;
    FAMILY *pFamCurr, *pFamHelp;
    THREAD_PARAM *aThrdParam;
    
    iRtnCode = 0;
    aThrd = NULL;
    aThrdParam = NULL;
    /* Create the output folder if it does not exist. */
    struct stat stDir = {0};
    if (stat(_pCfg->szPathOutput, &stDir) == -1) {
        iStatus = mkdir(_pCfg->szPathOutput, 0700);
        if (iStatus != 0) {
            Spew1("Error: %s.", strerror(errno));
            iRtnCode = -1;
            goto EXIT;
        }
    }
    
    /* Prepare the storge for extracted byte sequences of each group. */
    _aBin = pGrpRes->pABin;
    _uiGrpCnt = HASH_CNT(hh, pGrpRes->pMapFam);
    _aRawPtn = (UT_array**)malloc(sizeof(UT_array*) * _uiGrpCnt);
    if (_aRawPtn == NULL) {
        Spew0("Error: Cannot allocate the array for raw patterns.");
        iRtnCode = -1;
        goto EXIT;
    }
    for (uiIter = 0 ; uiIter < _uiGrpCnt ; uiIter++) {
        _aRawPtn[uiIter] = NULL;
    }
    
    /* Prepare the thread parameters. */
    aThrd = (pthread_t*)malloc(sizeof(pthread_t) * _uiGrpCnt);
    if (aThrd == NULL) {
        Spew0("Error: Cannot allocate the array for thread id.");
        iRtnCode = -1;
        goto EXIT;    
    }
    aThrdParam = (THREAD_PARAM*)malloc(sizeof(THREAD_PARAM) * _uiGrpCnt);
    if (aThrdParam == NULL) {
        Spew0("Error: Cannot allocate the array for thread parameters.");
        iRtnCode = -1;
        goto EXIT;
    }
    
    /* Prepare the thread semaphore. */
    sem_init(&_pSem, 0, _uiGrpCnt);
    
    /* Fork a dedicated thread to search the similar binary sequences for one group. 
       But the thread replication is controlled by the semphore with the quantity 
       limited by the parallelity configuration. */
    uiIter = 0;
    HASH_ITER (hh, pGrpRes->pMapFam, pFamCurr, pFamHelp) {
        sem_wait(&_pSem);
        aThrdParam[uiIter].uiThrdId = uiIter + 1;
        aThrdParam[uiIter].pFam = pFamCurr;
        pthread_create(&aThrd[uiIter], NULL, PtnGrepByteSequenceFromGroup, 
                       (void*)&(aThrdParam[uiIter]));
        uiIter++;
    }
    for (uiIter = 0 ; uiIter < _uiGrpCnt ; uiIter++) {
        pthread_join(aThrd[uiIter], NULL);
    }
    
    /* Release the thread semaphore. */
    sem_destroy(&_pSem);
    
EXIT:
    if (aThrd != NULL) {
        free(aThrd);
    } 
    if (aThrdParam != NULL) {
        free(aThrdParam);
    }
    
    return iRtnCode;
}

/**
 * !EXTERNAL
 * PtnGeneratePattern(): Output the set of candidates each of which is outputted 
 * as a Yara-formatted pattern.
 */
int PtnGeneratePattern(PATTERN *self) {
    int iRtnCode;
    uint8_t ucIterBlk, ucBlkCnt;
    uint32_t uiIterGrp, uiPtnBufSize, uiPtnLen, uiWrtCntStr, uiWrtCntCond;
    uint32_t uiOfstStr, uiOfstCond, uiOfstPtn;
    size_t nExptWrt, nRealWrt;
    bool bExceptMem, bExceptIo;
    char *bufPtn, *bufSectStr, *bufSectCond;
    FILE *fpPtn;
    SEQUENCE *pSeq;
    char szPathPtn[BUF_SIZE_MEDIUM];

    iRtnCode = 0;
    for (uiIterGrp = 0 ; uiIterGrp < _uiGrpCnt ; uiIterGrp++) {
        ucBlkCnt = MIN(_pCfg->ucBlkCnt, ARRAY_LEN(_aRawPtn[uiIterGrp]));
        bExceptMem = bExceptIo = false;
        fpPtn = NULL;
        
        /* Prepare the buffer to generate pattern. */
        bufPtn = bufSectStr = bufSectCond = NULL;
        bufSectStr = (char*)malloc(sizeof(char) * BUF_SIZE_LARGE);
        if (bufSectStr == NULL) {
            bExceptMem = true;
            goto FREE;
        }
        bufSectCond = (char*)malloc(sizeof(char) * BUF_SIZE_LARGE);
        if (bufSectCond == NULL) {
            bExceptMem = true;
            goto FREE;
        }
        uiPtnBufSize = (BUF_SIZE_LARGE) + (BUF_SIZE_LARGE) + (BUF_SIZE_MEDIUM);
        bufPtn = (char*)malloc(sizeof(char) * uiPtnBufSize);
        if (bufPtn == NULL) {
            bExceptMem = true;
            goto FREE;
        }
        
        /* Extract the byte sequences with the best quality:
           1. The hex byte strings to be showed in "strings" section. 
           2. The matching offsets to be showed in "conditions" section. */
        ucIterBlk = 0;
        uiOfstStr = uiOfstCond = 0;
        pSeq = NULL;
        while ((pSeq = (SEQUENCE*)ARRAY_NEXT(_aRawPtn[uiIterGrp], pSeq)) != NULL) {
            _PtnFormatSectionString(bufSectStr + uiOfstStr, &uiWrtCntStr, ucIterBlk,
                                    pSeq->aByte, pSeq->ucByteCnt);
            _PtnFormatSectionCondition(bufSectCond + uiOfstCond, &uiWrtCntCond, ucIterBlk,
                                       ucBlkCnt, pSeq->uiOfst, pSeq->pSetSectIdx);
            uiOfstStr += uiWrtCntStr;
            uiOfstCond += uiWrtCntCond;
            ucIterBlk++;
            if (ucIterBlk == ucBlkCnt) {
                break;
            }
        }
        bufSectStr[uiOfstStr] = 0;
        bufSectCond[uiOfstCond] = 0;

        /* Build the import section. */
        uiOfstPtn = sprintf(bufPtn, "import \"%s\"\n\n", MODULE_PE);
        
        /* Build the pattern title. */
        uiOfstPtn += sprintf(bufPtn + uiOfstPtn, "rule %s_%d\n", PREFIX_PATTERN,
                             uiIterGrp);
        bufPtn[uiOfstPtn++] = '{';
        bufPtn[uiOfstPtn++] = '\n';
        
        /* Build the "strings" section. */
        uiOfstPtn += sprintf(bufPtn + uiOfstPtn, "%s%sstrings:\n", SPACE_SUBS_TAB,
                             SPACE_SUBS_TAB);
        uiOfstPtn += sprintf(bufPtn + uiOfstPtn, "%s\n", bufSectStr);
        
        /* Build the "conditions" section. */
        uiOfstPtn += sprintf(bufPtn + uiOfstPtn, "%s%scondition:\n", SPACE_SUBS_TAB,
                             SPACE_SUBS_TAB);
        uiOfstPtn += sprintf(bufPtn + uiOfstPtn, "%s}\n", bufSectCond);
        
        /* Output the pattern. */
        sprintf(szPathPtn, "%s\%s_%d.yara", _pCfg->szPathOutput, PREFIX_PATTERN,
                uiIterGrp);
               
        fpPtn = fopen(szPathPtn, "w");
        if (fpPtn == NULL) {
            bExceptIo = true;
            goto FREE;
        }
        nExptWrt = uiOfstPtn;
        nRealWrt = fwrite(bufPtn, sizeof(char), nExptWrt, fpPtn);
        if (nExptWrt != nRealWrt) {
            bExceptIo = true;
            goto FREE;
        }
        fclose(fpPtn);
    FREE:
        if (bufSectStr != NULL) {
            free(bufSectStr);
        }
        if (bufSectCond != NULL) {
            free(bufSectCond);
        }
        if (bufPtn != NULL) {
            free(bufPtn);
        }
        if (bExceptMem == true) {
            Spew0("Error: Cannot allocate buffer for pattern generation.");
            iRtnCode = -1;
            break;
        }
        if (bExceptIo == true) {
            Spew1("Error: %s.", strerror(errno));
            iRtnCode = -1;
            if (fpPtn != NULL) {
                fclose(fpPtn);
            }
            break;
        }
    }

    return iRtnCode;
}


/*======================================================================*
 *                Implementation for Internal Functions                 *
 *======================================================================*/
/**
 * !INTERNAL
 * PtnGrepByteSequenceFromGroup(): Extract the similar byte sequences shared 
 * by the given group of sections.
 */
static void* PtnGrepByteSequenceFromGroup(void *vpThrdParam) {
    uint8_t ucBlkSize, ucIterCmp;
    uint32_t uiThrdId, uiIterSlot, uiIterSeq;
    uint32_t uiGrpSize, uiSlotCnt, uiSeqCnt, uiMinSeqCnt;
    SEQUENCE seqInst;
    THREAD_PARAM *pThrdParam;
    UT_array *aSeq, *aFamMbr;
    SEQUENCE *pSeqSrc, *pSeqTge;
    UT_array **aASeq;
    
    pThrdParam = (THREAD_PARAM*)vpThrdParam;
    uiThrdId = pThrdParam->uiThrdId;
    aFamMbr = pThrdParam->pFam->aFamMbr;
    uiGrpSize = ARRAY_LEN(aFamMbr);    
    uiSlotCnt = (uint32_t)ceil((double)uiGrpSize / _pCfg->ucIoBandwidth);
    aASeq = NULL;
    aASeq = (UT_array**)malloc(sizeof(UT_array*) * uiSlotCnt);
    if (aASeq == NULL) {
        Spew0("Error: Cannot allocate the array of SEQUENCEs for job slots.");
        goto EXIT;   
    }    

    /* Evenly divide the clustered sections into slots. For each slot, an array of 
       SEQUENCE structures is preared to record the similar byte sequences shared by 
       the sections belonged to that slot. */
    UT_icd icdSeq = { sizeof(SEQUENCE), NULL, UTArraySequenceCopy, UTArraySequenceDeinit};
    uiMinSeqCnt = UINT_MAX;
    for (uiIterSlot = 0 ; uiIterSlot < uiSlotCnt ; uiIterSlot++) {
        aASeq[uiIterSlot] = NULL;
        ARRAY_NEW(aASeq[uiIterSlot], &icdSeq);
        _PtnGrepByteSequencePerSlot(aFamMbr, uiGrpSize, uiIterSlot, aASeq[uiIterSlot]);
        uiSeqCnt = ARRAY_LEN(aASeq[uiIterSlot]);
        if (uiSeqCnt < uiMinSeqCnt) {
            uiMinSeqCnt = uiSeqCnt;
        }
    }

    /* Merge the byte sequences extracted from each slot. */
    ucBlkSize = _pCfg->ucBlkSize;
    for (uiIterSeq = 0 ; uiIterSeq < uiMinSeqCnt ; uiIterSeq++) {
        pSeqSrc = (SEQUENCE*)ARRAY_ELTPTR(aASeq[0], uiIterSeq);
        for (uiIterSlot = 0 ; uiIterSlot < uiSlotCnt ; uiIterSlot++) {
            pSeqTge = (SEQUENCE*)ARRAY_ELTPTR(aASeq[uiIterSlot], uiIterSeq);
            for (ucIterCmp = 0 ; ucIterCmp < ucBlkSize ; ucIterCmp++) {
                if (pSeqSrc->aByte[ucIterCmp] == DONT_CARE_MARK) {
                    continue;
                }
                if (pSeqSrc->aByte[ucIterCmp] != pSeqTge->aByte[ucIterCmp]) {
                    pSeqSrc->aByte[ucIterCmp] = DONT_CARE_MARK;
                    pSeqSrc->ucDontCareCnt++;
                }
            }
        }
    }
    
    /* Store the merged result into the array of raw patterns. */
    ARRAY_SORT(aASeq[0], UTArraySequenceSort);
    _aRawPtn[uiThrdId - 1] = aASeq[0];

EXIT:
    if (aASeq != NULL) {
        for (uiIterSlot = 1 ; uiIterSlot < uiSlotCnt ; uiIterSlot++) {
            if (aASeq[uiIterSlot] != NULL) {
                ARRAY_FREE(aASeq[uiIterSlot]);
            }
        }
        free(aASeq);        
    }
    
    /* Resume the thread creater. */
    sem_post(&_pSem);
    printf("=== Thread %3d === Finish the job.\n", uiThrdId);

    return;
}

/**
 * !INTERNAL
 * _PtnGrepByteSequencePerSlot(): Extract the byte sequences for the given slot 
 * and deliver the result to PtnGrepByteSequenceFromGroup() for merging.
 */
static int _PtnGrepByteSequencePerSlot(UT_array *aFamMbr, uint32_t uiGrpSize,
                                       uint32_t uiIdxSlot, UT_array *aSeq) {
    int iRtnCode, iStatus;
    uint8_t ucIterCmp, ucBlkSize, ucDcCnt;
    uint32_t uiIterBin, uiIterRnd, uiIdxBin, uiIdxBgn, uiIdxEnd;
    uint32_t uiOfst, uiBinCnt, uiMinBinSize, uiRound;
    uint16_t *bufCmp, *aSectIdx;
    uint32_t *pUiIdxBin;
    size_t nExptRead, nRealRead;
    bool bExceptIo;
    FILE *fpSample;
    BINARY *pBin;
    SEQUENCE seqInst;
    SECTION_SET *pSetCurr;
    char **aBinContent;
    char szPathSample[BUF_SIZE_MEDIUM];

    iRtnCode = 0;
    uiIdxBgn = uiIdxSlot * _pCfg->ucIoBandwidth;
    uiIdxEnd = MIN(uiIdxBgn + _pCfg->ucIoBandwidth, uiGrpSize);
    uiBinCnt = uiIdxEnd - uiIdxBgn;
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
    aBinContent = (char**)malloc(sizeof(char*) * uiBinCnt);
    if (aBinContent == NULL) {
        Spew0("Error: Cannot allocate the array for section contents.");
        iRtnCode = -1;
        goto EXIT;
    }
    aSectIdx = (uint16_t*)malloc(sizeof(uint16_t) * uiBinCnt);
    if (aSectIdx == NULL) {
        Spew0("Error: Cannot allocate the array for section sizes.");
        iRtnCode = -1;
        goto EXIT;
    }
    for (uiIterBin = 0 ; uiIterBin < uiBinCnt ; uiIterBin++) {
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
        bExceptIo= false;
        iStatus = fseek(fpSample, pBin->uiSectOfst, SEEK_SET);
        if (iStatus != 0) {
            Spew1("Error: %s.", strerror(errno));
            bExceptIo = true;
            goto CLOSE;
        }
        nRealRead = fread(aBinContent[uiIdxBin], sizeof(char), nExptRead, fpSample);
        if (nRealRead != nExptRead) {
            Spew1("Error: %s.", strerror(errno));
            bExceptIo = true;
            goto CLOSE;
        }
        
    CLOSE:
        fclose(fpSample);
        if (bExceptIo == true) {
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
            bufCmp[ucIterCmp] = aBinContent[0][uiOfst + ucIterCmp] & EXTENSION_MASK;    
        }
        
        /* Compare the rest of the sections with the source comparator. */
        ucDcCnt = 0;
        for (uiIterBin = 1 ; uiIterBin < uiBinCnt ; uiIterBin++) {
            for (ucIterCmp = 0 ; ucIterCmp < ucBlkSize ; ucIterCmp++) {
                if (bufCmp[ucIterCmp] == DONT_CARE_MARK) {
                    continue;
                }
                if (bufCmp[ucIterCmp] != aBinContent[uiIterBin][uiOfst + ucIterCmp]) {
                    bufCmp[ucIterCmp] = DONT_CARE_MARK;
                    ucDcCnt++;
                }
            }
        }
        
        seqInst.ucDontCareCnt = ucDcCnt;
        seqInst.ucByteCnt = ucBlkSize;
        seqInst.uiOfst = uiOfst;

        /* Record the indices of sections contributing to the extracted byte sequence.*/
        seqInst.pSetSectIdx = NULL;
        for (uiIterBin = 0 ; uiIterBin < uiBinCnt ; uiIterBin++) {
            HASH_FIND(hh, seqInst.pSetSectIdx, &aSectIdx[uiIterBin], sizeof(uint16_t), pSetCurr);
            if (pSetCurr == NULL) {
                pSetCurr = (SECTION_SET*)malloc(sizeof(SECTION_SET));
                if (pSetCurr == NULL) {
                    Spew0("Cannot allocate SECTION_SET for section index recording.");
                    iRtnCode = -1;
                    goto EXIT;
                }
                pSetCurr->usSectIdx = aSectIdx[uiIterBin];
                HASH_ADD(hh, seqInst.pSetSectIdx, usSectIdx, sizeof(uint16_t), pSetCurr);
            }       
        }
        
        /* Record the byte sequence. */
        seqInst.aByte = (uint16_t*)malloc(sizeof(uint16_t*) * ucBlkSize);
        if (seqInst.aByte == NULL) {
            Spew0("Cannot allocate buffer for extracted byte sequence.");
            iRtnCode = -1;
            goto EXIT;
        }
        for (ucIterCmp = 0 ; ucIterCmp < ucBlkSize ; ucIterCmp++) {
            seqInst.aByte[ucIterCmp] = bufCmp[ucIterCmp];
        }
        
        ARRAY_PUSH_BACK(aSeq, &seqInst);
    }

EXIT:
    if (bufCmp != NULL) {
        free(bufCmp);
    }
    if (aBinContent != NULL) {
        for (uiIterBin = 0 ; uiIterBin < uiBinCnt ; uiIterBin++) {
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

/**
 * !INTERNAL
 * _PtnFormatSectionString(): Format the hex byte sequences to be showed in the 
 * "string" section of YARA pattern.
 */
static void _PtnFormatSectionString(char *bufBgn, uint32_t *pUiWrtCnt,
                                    uint8_t ucIdxBlk, uint16_t *aByte, 
                                    uint8_t ucByteCnt) {    
    uint8_t ucIdxByte, ucIndentLen, ucIdxIndent;
    uint32_t uiWrtCnt;
    char bufIndent[BUF_SIZE_SMALL];
    
    uiWrtCnt = sprintf(bufBgn, "%s%s$%s_%d = { ", SPACE_SUBS_TAB, SPACE_SUBS_TAB, 
                       PREFIX_HEX_STRING, ucIdxBlk);
    
    /* Prepare the indent space. */
    ucIndentLen = uiWrtCnt;
    memset(bufIndent, 0, sizeof(char) * BUF_SIZE_SMALL);
    for (ucIdxIndent = 0 ; ucIdxIndent < ucIndentLen ; ucIdxIndent++) {
        bufIndent[ucIdxIndent] = ' ';
    }
                        
    for (ucIdxByte = 0 ; ucIdxByte < ucByteCnt ; ucIdxByte++) {
        if (aByte[ucIdxByte] != DONT_CARE_MARK) {
            uiWrtCnt += sprintf(bufBgn + uiWrtCnt, "%02x ", aByte[ucIdxByte]);
        } else {
            bufBgn[uiWrtCnt] = '?';
            bufBgn[uiWrtCnt + 1] = '?';
            bufBgn[uiWrtCnt + 2] = ' ';
            uiWrtCnt += 3;
        }
        /* Newline if the number of written bytes exceeding the threshold. */
        if ((ucIdxByte % HEX_CHUNK_SIZE == HEX_CHUNK_SIZE - 1) &&
            (ucIdxByte != ucByteCnt - 1)) {
            uiWrtCnt += sprintf(bufBgn + uiWrtCnt, "\n%s", bufIndent);
        }
    }
    bufBgn += uiWrtCnt;
    bufBgn[0] = '}';
    bufBgn[1] = '\n';
    bufBgn[2] = '\n';
    *pUiWrtCnt = uiWrtCnt + 3;

    return;
}

/**
 * !INTERNAL
 * _PtnFormatSectionCondition(): Format the matching offset offsets to be showed 
 * in the "conditions" section of YARA pattern.
 */
static void _PtnFormatSectionCondition(char *bufBgn, uint32_t *pUiWrtCnt,
                                       uint8_t ucIdxBlk, uint8_t ucBlkCnt, 
                                       uint32_t uiOfst, SECTION_SET *pSetSectIdx) {
    uint16_t usIterSect, usSectCnt;    
    uint32_t uiWrtCnt;
    SECTION_SET *pSetCurr, *pSetHelp;    
    
    uiWrtCnt = 0;
    usSectCnt = HASH_COUNT(pSetSectIdx);
    usIterSect = 1;
    HASH_ITER(hh, pSetSectIdx, pSetCurr, pSetHelp) {
        uiWrtCnt += sprintf(bufBgn + uiWrtCnt, 
                            "%s%s$%s_%d at pe.sections[%d].raw_data_offset + 0x%x",
                            SPACE_SUBS_TAB, SPACE_SUBS_TAB, PREFIX_HEX_STRING, 
                            ucIdxBlk, pSetCurr->usSectIdx, uiOfst);
        if (usIterSect < usSectCnt) {
            bufBgn[uiWrtCnt] = ' ';
            bufBgn[uiWrtCnt + 1] = 'o';
            bufBgn[uiWrtCnt + 2] = 'r';
            uiWrtCnt += 3;
        }
        bufBgn[uiWrtCnt++] = '\n';
        usIterSect++;
    }
    if ((ucIdxBlk + 1) < ucBlkCnt) {
        uiWrtCnt += sprintf(bufBgn + uiWrtCnt, "%s%sor\n", SPACE_SUBS_TAB, SPACE_SUBS_TAB);
    } else {
        bufBgn[uiWrtCnt++] = '\n';
    }
    *pUiWrtCnt = uiWrtCnt;

    return;
}
