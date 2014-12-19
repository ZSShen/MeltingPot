#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <glib.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include "spew.h"
#include "data.h"
#include "cluster.h"
#include "pattern.h"


static sem_t synSem;
static CONFIG *p_Conf;
static MELT_POT *p_Pot;


/*======================================================================*
 *                 Declaration for Internal Functions                   *
 *======================================================================*/
/**
 * This function extracts a set of commonly shared byte sequences for a group.
 *
 * @param vp_Param      The THREAD_CRAFT type parameter.
 *
 * @return (currently deprecated)
 */
void*
_PtnMapCraft(void *vp_Param);


/**
 * This function extracts the commonly shared byte sequences for the samples
 * belonged to the same partition slot.
 * 
 * @param p_Param       The pointer to the THREAD_SLOT parameter.
 * 
 * @return status code
 */
int8_t
_PtnMapSlot(THREAD_SLOT *p_Param);


/**
 * This function iteratively merges the results extracted from each group.
 *
 * @param p_Param       The pointer to the updated THREAD_CRAFT parameter.
 * 
 * @return status code
 */
int8_t
_PtnReduceCraft(THREAD_CRAFT *p_Param);


/**
 * This function iteratively merges the results extracted from each slot.
 * 
 * @param a_Param       The array of the updated THREAD_SLOT parameter.
 * @param ulSize        The array size
 * 
 * @return status code
 */
int8_t
_PtnReduceSlot(THREAD_SLOT *a_Param, uint64_t ulSize);


/**
 * This function prints the pattern file for the designated group.
 * 
 * @param ulIdxGrp      The group index.
 * @param ulSizeGrp     The group size.
 * @param a_BlkCand     The array of BLOCK_CAND structures.
 * 
 * @return status code
 */
int8_t
_PtnPrintf(uint64_t ulIdxGrp, uint64_t ulSizeGrp, GPtrArray *a_BlkCand);


/**
 * This function prints the string section of the pattern.
 * 
 * @param szPtnStr      The string showing the content of string section.
 * @param a_usCtn       The buffer storing the block hex bytes.
 * @param p_sLen        The pointer to the to be updated section length.
 * @param ucIdxBlk      The index of the hex block.
 * 
 * @return status code
 */
int8_t
_PtnPrintStringSection(char *szPtnStr, uint16_t *a_usCtn, int16_t *p_sLen, 
                       uint8_t ucIdxBlk);


/**
 * This function prints the condition section of the pattern.
 * 
 * @param szPtnCond     The string showing the content of condition section.
 * @param a_CtnAddr     The array of CONTENT_ADDR structures.
 * @param p_sLen        The pointer to the to be updated section length.
 * @param ucCntBlk      The total number of hex blocks.
 * @param ucIdxBlk      The index of the hex block.
 * 
 * @return status code
 */
int8_t
_PtnPrintConditionSection(char *szPtnCond, GArray *a_CtnAddr, int16_t *p_sLen,
                          uint8_t ucCntBlk, uint8_t ucIdxBlk);


/**
 * This function initializes the THREAD_CRAFT structure.
 * 
 * @param p_Param       The pointer to the to be initialized structure.
 * @param p_Grp         The pointer to the GROUP structure.
 * 
 * @return status code (currently unused)
 */
int8_t
_PtnSetParamThrdCrt(THREAD_CRAFT *p_Param, GROUP *p_Grp);


/**
 * This function initializes the THREAD_SLOT structure.
 * 
 * @param p_Param       The pointer to the to be initialized structure.
 * @param p_Grp         The pointer to the GROUP structure.
 * @param ulIdxBgn      The beginning index to the assigned range.
 * @param ulIdxEnd      The ending index to the assigned range.
 * 
 * @return status code
 */
int8_t
_PtnSetParamThrdSlt(THREAD_SLOT *p_Param, GROUP *p_Grp, uint64_t ulIdxBgn, uint64_t ulIdxEnd);


/**
 * This function initializes the array of THREAD_CRAFT structures.
 * 
 * @param p_aParam      The pointer to the array of THREAD_CRAFT structures.
 * @param ulSize        The array size.
 * 
 * @return status code
 */
int8_t
_PtnInitArrayThrdCrt(THREAD_CRAFT **p_aParam, uint64_t ulSize);


/**
 * This function initializes the array of THREAD_SLOT structures.
 * 
 * @param p_aParam      The pointer to the array of THREAD_SLOT structures.
 * @param ulSize        The array size.
 * 
 * @return status code
 */
int8_t
_PtnInitArrayThrdSlt(THREAD_SLOT **p_aParam, uint64_t ulSize);


/**
 * This function deinitializes the array of THREAD_CRAFT structures.
 * 
 * @param a_Param       The array of THREAD_CRAFT structures.
 * @param ulSize        The array size.
 * 
 * @return (currently unused)
 */
int8_t
_PtnDeinitArrayThrdCrt(THREAD_CRAFT *a_Param, uint64_t ulSize);


/**
 * This function deinitializes the array of THREAD_SLOT structures.
 * 
 * @param a_Param       The array of THREAD_SLOT structures.
 * @param ulSize        The array size.
 * 
 * @return (currently unused)
 */
int8_t
_PtnDeinitArrayThrdSlt(THREAD_SLOT *a_Param, uint64_t ulSize);


/*======================================================================*
 *                Implementation for Exported Functions                 *
 *======================================================================*/
int8_t
PtnSetContext(CONTEXT *p_Ctx)
{
    p_Conf = p_Ctx->p_Conf;
    p_Pot = p_Ctx->p_Pot;
    
    return CLS_SUCCESS;
}


int8_t
PtnCraftPattern()
{
    int8_t cRtnCode = CLS_SUCCESS;

    /*-----------------------------------------------------------------------*
     * Spawn multipe threads each of which:                                  *
     * 1. Extract a set of commonly shared byte blocks with user specified   *
     *    quality for the given group.                                       *
     *-----------------------------------------------------------------------*/
    THREAD_CRAFT *a_Param;
    uint64_t ulCntGrp = g_hash_table_size(p_Pot->h_Grp);
    int8_t cStat = _PtnInitArrayThrdCrt(&a_Param, ulCntGrp);
    if (cStat != CLS_SUCCESS)
        EXITQ(cStat, EXIT);

    sem_init(&synSem, 0, p_Conf->ucCntThrd);
    GHashTableIter iterHash;
    gpointer gpKey, gpValue;
    uint64_t ulIdx = 0;
    g_hash_table_iter_init(&iterHash, p_Pot->h_Grp);
    while (g_hash_table_iter_next(&iterHash, &gpKey, &gpValue)) {
        /* Discard the trivial groups using the designated threshold. */
        GROUP *p_Grp = (GROUP*)gpValue;
        if (p_Grp->a_Mbr->len < p_Conf->ucSizeTruncGrp)
            continue;
        sem_wait(&synSem);
        _PtnSetParamThrdCrt(&(a_Param[ulIdx]), p_Grp);
        pthread_create(&(a_Param[ulIdx].tId), NULL, _PtnMapCraft, (void*)&(a_Param[ulIdx]));
        ulIdx++;
    }

    /*-----------------------------------------------------------------------*
     * Join the spawned threads:                                             *
     * 1. Merge the byte block sets collected from each group.               * 
     *-----------------------------------------------------------------------*/
    uint64_t ulCntFork = ulIdx;
    for (ulIdx = 0 ; ulIdx < ulCntFork ; ulIdx++) {
        pthread_join(a_Param[ulIdx].tId, NULL);
        _PtnReduceCraft(&(a_Param[ulIdx]));
        if (a_Param[ulIdx].cRtnCode != CLS_SUCCESS)
            cRtnCode = CLS_FAIL_PROCESS;
    }
    sem_destroy(&synSem);

FREEPARAM:    
    _PtnDeinitArrayThrdCrt(a_Param, ulCntGrp);  
EXIT:
    return cRtnCode;
}


int8_t
PtnOutputResult()
{
    int8_t cRtnCode = CLS_SUCCESS;

    /* Create the YARA format pattern for each group. */
    GHashTableIter iterHash;
    gpointer gpKey, gpValue;
    g_hash_table_iter_init(&iterHash, p_Pot->h_Grp);
    uint64_t ulIdxGrp = 0;
    while (g_hash_table_iter_next(&iterHash, &gpKey, &gpValue)) {
        GROUP *p_Grp = (GROUP*)gpValue;
        GArray *a_Mbr = p_Grp->a_Mbr;
        GPtrArray *a_BlkCand = p_Grp->a_BlkCand;
        uint64_t ulSizeGrp = a_Mbr->len;
        if (a_BlkCand->len == 0)
            continue;
        int8_t cStat = _PtnPrintf(ulIdxGrp, ulSizeGrp, a_BlkCand);
        if (cStat != CLS_SUCCESS)
            EXITQ(cStat, EXIT);
        ulIdxGrp++;    
    }

EXIT:
    return cRtnCode;
}


/*======================================================================*
 *                Implementation for Internal Functions                 *
 *======================================================================*/
void*
_PtnMapCraft(void *vp_Param)
{
    int8_t cRtnCode = CLS_SUCCESS;
    
    THREAD_CRAFT *p_Param = (THREAD_CRAFT*)vp_Param;
    GROUP *p_Grp = p_Param->p_Grp;
    GArray *a_Mbr = p_Grp->a_Mbr;

    /*-----------------------------------------------------------------------*
     * To avoid opening too many files simultaneously, we split the grouped  *
     * slices into sevaral slots. And we first extract the commonly shared   *
     * byte blocks from each slot and then merge them together.              *
     *-----------------------------------------------------------------------*/
    uint64_t ulCntMbr = a_Mbr->len;
    uint64_t ulCntSlot = (uint64_t)ceil((double)ulCntMbr / p_Conf->ucIoBand);
    THREAD_SLOT *a_Param;
    int8_t cStat = _PtnInitArrayThrdSlt(&a_Param, ulCntSlot);    
    if (cStat != CLS_SUCCESS)
        EXITQ(cStat, EXIT);

    uint64_t ulIdxSlot, ulIdxBgn, ulIdxEnd = 0;
    for (ulIdxSlot = 0 ; ulIdxSlot < ulCntSlot ; ulIdxSlot++) {
        ulIdxBgn = ulIdxEnd;
        ulIdxEnd = MIN(ulIdxBgn + p_Conf->ucIoBand, ulCntMbr);
        cStat = _PtnSetParamThrdSlt(&(a_Param[ulIdxSlot]), p_Grp, ulIdxBgn, ulIdxEnd);
        if (cStat != CLS_SUCCESS)
            break;
        cStat = _PtnMapSlot(&(a_Param[ulIdxSlot]));
        if (cStat != CLS_SUCCESS)
            break;
    }

    _PtnReduceSlot(a_Param, ulCntSlot);

FREEPARAM:
    _PtnDeinitArrayThrdSlt(a_Param, ulCntSlot);
EXIT:
    p_Param->cRtnCode = cRtnCode;
    sem_post(&synSem);
    return;
}


int8_t
_PtnMapSlot(THREAD_SLOT *p_Param)
{
    int8_t cRtnCode = CLS_SUCCESS;

    uint8_t ucSizeBlk = p_Conf->ucSizeBlk;
    char **a_szBin = p_Param->a_szBin;
    GArray *a_Mbr = p_Param->a_Mbr;
    uint64_t ulIdxBgn = p_Param->ulIdxBgn;
    uint64_t ulIdxEnd = p_Param->ulIdxEnd;
    uint16_t usFront = 0;
    uint16_t usRear = ucSizeBlk;
    uint64_t ulRange = ulIdxEnd - ulIdxBgn;

    while (usRear <= p_Param->usSizeMinSlc) {
        BLOCK_CAND *p_BlkCand;
        uint8_t cStat = DsNewBlockCand(&p_BlkCand, ucSizeBlk);
        if (cStat != CLS_SUCCESS)
            EXITQ(cStat, EXIT);

        /* Let the first slice be the comparison base. */
        uint16_t *a_usCtn = p_BlkCand->a_usCtn;
        GArray *a_CtnAddr = p_BlkCand->a_CtnAddr;
        uint16_t usSrc, usTge;
        for (usSrc = usFront, usTge = 0 ; usSrc < usRear ; usSrc++, usTge++)
            a_usCtn[usTge] = a_szBin[0][usSrc];

        uint64_t ulIdSlc = g_array_index(a_Mbr, uint64_t, ulIdxBgn);
        SLICE *p_Slc = g_ptr_array_index(p_Pot->a_Slc, ulIdSlc);
        CONTENT_ADDR addr;
        addr.iIdSec = p_Slc->iIdSec;
        addr.ulOfstRel = p_Slc->ulOfstRel + usFront;
        g_array_append_val(a_CtnAddr, addr);

        /* Iteratively compare the rest slices with the base. */
        uint64_t ulOfst, ulIdx;
        for (ulOfst = 1, ulIdx = ulIdxBgn + 1; ulOfst < ulRange ; ulOfst++, ulIdx++) {
            for (usSrc = usFront, usTge = 0 ; usSrc < usRear ; usSrc++, usTge++) {
                if (a_usCtn[usTge] != a_szBin[ulOfst][usSrc])
                    a_usCtn[usTge] = WILD_CARD_MARK;
            }

            ulIdSlc = g_array_index(a_Mbr, uint64_t, ulIdx);
            p_Slc = g_ptr_array_index(p_Pot->a_Slc, ulIdSlc);
            addr.iIdSec = p_Slc->iIdSec;
            addr.ulOfstRel = p_Slc->ulOfstRel + usFront;
            g_array_append_val(a_CtnAddr, addr);
        }

        g_ptr_array_add(p_Param->a_BlkCand, p_BlkCand);
        usFront += ROLL_SHFT_COUNT;
        usRear += ROLL_SHFT_COUNT;
    }

EXIT:
    return cRtnCode;
}


int8_t
_PtnReduceCraft(THREAD_CRAFT *p_Param)
{
    GPtrArray *a_BlkCand = p_Param->p_Grp->a_BlkCand;

    /* Remove the blocks with the number of noisy bytes exceeding the threshold. */
    g_ptr_array_sort(a_BlkCand, DsCompBlockCandNoise);
    uint8_t ucThld = p_Conf->ucSizeBlk * p_Conf->ucRatNoise / THLD_DNMNTR;
    uint32_t uiLen = a_BlkCand->len;
    uint32_t uiIdx;
    for (uiIdx = 0 ; uiIdx < uiLen ; uiIdx++) {
        BLOCK_CAND *p_BlkCand = g_ptr_array_index(a_BlkCand, uiIdx);
        if (p_BlkCand->ucCntNoise >= ucThld)
            break;
    }
    if (uiIdx < uiLen)
        g_ptr_array_remove_range(a_BlkCand, uiIdx, (uiLen - uiIdx));

    /* Early return if all the blocks are eliminated. */
    if (a_BlkCand->len == 0)
        return CLS_SUCCESS;

    /* Sort the blocks and retrieve the candidates with top quality. */
    g_ptr_array_sort(a_BlkCand, DsCompBlockCandWildCard);
    ucThld = p_Conf->ucSizeBlk * p_Conf->ucRatWild / THLD_DNMNTR;
    uiLen = a_BlkCand->len;
    for (uiIdx = 0 ; uiIdx < uiLen ; uiIdx++) {
        BLOCK_CAND *p_BlkCand = g_ptr_array_index(a_BlkCand, uiIdx);
        if (p_BlkCand->ucCntWild >= ucThld)
            break;
        if (uiIdx == p_Conf->ucCntBlk)
            break;   
    }
    g_ptr_array_remove_range(a_BlkCand, uiIdx, (uiLen - uiIdx));

    return CLS_SUCCESS;
}


int8_t
_PtnReduceSlot(THREAD_SLOT *a_Param, uint64_t ulSize)
{
    int8_t cRtnCode = CLS_SUCCESS;

    uint16_t usCntMin = USHRT_MAX;
    uint64_t ulIdx;
    for (ulIdx = 0 ; ulIdx < ulSize ; ulIdx++) {
        if (a_Param[ulIdx].a_BlkCand->len < usCntMin)
            usCntMin = a_Param[ulIdx].a_BlkCand->len;
    }

    /* Let the common block list extracted from the first slot be the comparison base. */
    THREAD_SLOT *p_Param = a_Param;
    GPtrArray *a_BlkCandB = p_Param->a_BlkCand;

    /* Iteratively compare the rest block lists with the base. */
    uint16_t usIdx;
    for (usIdx = 0 ; usIdx < usCntMin ; usIdx++) {
        BLOCK_CAND *p_BlkCandB = g_ptr_array_index(a_BlkCandB, usIdx);
        uint16_t *a_usCtnB = p_BlkCandB->a_usCtn;
        GArray *a_CtnAddrB = p_BlkCandB->a_CtnAddr;
        for (ulIdx = 1 ; ulIdx < ulSize ; ulIdx++) {
            p_Param = a_Param + ulIdx;
            GPtrArray *a_BlkCandC = p_Param->a_BlkCand;
            BLOCK_CAND *p_BlkCandC = g_ptr_array_index(a_BlkCandC, usIdx);
            uint16_t *a_usCtnC = p_BlkCandC->a_usCtn;
            GArray *a_CtnAddrC = p_BlkCandC->a_CtnAddr;

            uint8_t ucIdx;
            for (ucIdx = 0 ; ucIdx < p_Conf->ucSizeBlk ; ucIdx++) {
                if (a_usCtnB[ucIdx] != a_usCtnC[ucIdx])
                    a_usCtnB[ucIdx] = WILD_CARD_MARK;
            }

            uint32_t uiLen = a_CtnAddrC->len;
            uint32_t uiIdx;
            for (uiIdx = 0 ; uiIdx < uiLen ; uiIdx++) {
                CONTENT_ADDR addr = g_array_index(a_CtnAddrC, CONTENT_ADDR, uiIdx);
                g_array_append_val(a_CtnAddrB, addr);
            }
        }

        uint8_t ucIdx;
        for (ucIdx = 0 ; ucIdx < p_Conf->ucSizeBlk ; ucIdx++) {
            uint16_t usMat = a_usCtnB[ucIdx];
            if ((usMat == BYTE_NOISE_00) || (usMat == BYTE_NOISE_FF))
                p_BlkCandB->ucCntNoise++;
            if (usMat == WILD_CARD_MARK)
                p_BlkCandB->ucCntWild++;
        }
    }

EXIT:
    return cRtnCode;
}


int8_t
_PtnPrintf(uint64_t ulIdxGrp, uint64_t ulSizeGrp, GPtrArray *a_BlkCand)
{
    int8_t cRtnCode = CLS_SUCCESS;

    int32_t iLen = strlen(p_Conf->szPathRootOut) + DIGIT_COUNT_ULONG + 2;
    char *szPath = (char*)malloc(sizeof(char) * iLen);
    if (!szPath)
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));
    snprintf(szPath, iLen, "%s/%lu_%lu.yara", p_Conf->szPathRootOut, ulSizeGrp, ulIdxGrp);
    
    char *szPtnFull = (char*)malloc(sizeof(char) * BUF_SIZE_PTN_FILE);
    if (!szPtnFull)
        EXIT1(CLS_FAIL_MEM_ALLOC, FREEPATH, "Error: %s.", strerror(errno));

    char *szPtnStr = (char*)malloc(sizeof(char) * BUF_SIZE_PTN_SECTION);
    if (!szPtnStr)
        EXIT1(CLS_FAIL_MEM_ALLOC, FREEPTN_FULL, "Error: %s.", strerror(errno));

    char *szPtnCond = (char*)malloc(sizeof(char) * BUF_SIZE_PTN_SECTION);
    if (!szPtnCond)
        EXIT1(CLS_FAIL_MEM_ALLOC, FREEPTN_STR, "Error: %s.", strerror(errno));

    FILE *fp = fopen(szPath, "w");
    if (!fp)
        EXIT1(CLS_FAIL_FILE_IO, FREEPTN_COND, "Error: %s.", strerror(errno));

    /* Print the string and condition section. */
    int16_t iLenStr = 0;
    int16_t iLenCond = 0;
    uint8_t ucCntBlk = a_BlkCand->len;
    uint8_t ucIdx;
    for (ucIdx = 0 ; ucIdx < ucCntBlk ; ucIdx++) {
        BLOCK_CAND *p_BlkCand = g_ptr_array_index(a_BlkCand, ucIdx);

        uint16_t *a_usCtn = p_BlkCand->a_usCtn;
        int8_t cStat = _PtnPrintStringSection(szPtnStr, a_usCtn, &iLenStr, ucIdx);
        if (cStat != CLS_SUCCESS)
            EXITQ(cStat, CLOSE);
        szPtnStr[iLenStr] = 0;

        GArray *a_CtnAddr = p_BlkCand->a_CtnAddr;
        cStat = _PtnPrintConditionSection(szPtnCond, a_CtnAddr, &iLenCond, ucCntBlk, ucIdx);
        if (cStat != CLS_SUCCESS)
            EXITQ(cStat, CLOSE);
        szPtnCond[iLenCond] = 0;    
    }

    /* Print the header section. */
    char *szPivot = szPtnFull;
    int16_t sRest = BUF_SIZE_PTN_FILE;
    int16_t sCntWrt = snprintf(szPivot, sRest, "import \"%s\"\n\n", MODULE_PE);
    szPivot += sCntWrt;
    sRest -= sCntWrt;

    sCntWrt = snprintf(szPivot, sRest, "rule %s_%ld\n{\n", PREFIX_PATTERN, ulIdxGrp);
    szPivot += sCntWrt;
    sRest -= sCntWrt;

    sCntWrt = snprintf(szPivot, sRest, "%s%sstrings:\n%s\n", SPACE_SUBS_TAB, 
                       SPACE_SUBS_TAB, szPtnStr);
    szPivot += sCntWrt;
    sRest -= sCntWrt;

    sCntWrt = snprintf(szPivot, sRest, "%s%scondition:\n%s}\n", SPACE_SUBS_TAB,
                       SPACE_SUBS_TAB, szPtnCond);
    szPivot += sCntWrt;
    
    size_t nWrtExpt = szPivot - szPtnFull;
    size_t nWrtReal = fwrite(szPtnFull, sizeof(char), nWrtExpt, fp);
    if (nWrtReal != nWrtExpt)
        EXIT1(CLS_FAIL_FILE_IO, CLOSE, "Error: %s.", strerror(errno));

CLOSE:
    if (fp)
        fclose(fp);
FREEPTN_COND:
    if (szPtnCond)
        free(szPtnCond);
FREEPTN_STR:
    if (szPtnStr)
        free(szPtnStr);
FREEPTN_FULL:
    if (szPtnFull)
        free(szPtnFull);
FREEPATH:
    if (szPath)
        free(szPath);
EXIT:
    return cRtnCode;
}


int8_t
_PtnPrintStringSection(char *szPtnStr, uint16_t *a_usCtn, int16_t *p_sLen, 
                       uint8_t ucIdxBlk)
{
    int8_t cRtnCode = CLS_SUCCESS;

    char *szPivot = szPtnStr + *p_sLen;
    int16_t sRest = BUF_SIZE_PTN_SECTION - *p_sLen;
    if (sRest <= 0)
        EXIT1(CLS_FAIL_PTN_CREATE, EXIT, "Error: %s.", FAIL_PTN_CREATE);
    
    int8_t cCntWrt = snprintf(szPivot, sRest, "%s%s$%s_%d = { ", SPACE_SUBS_TAB,
                              SPACE_SUBS_TAB, PREFIX_HEX_STRING, ucIdxBlk);
    szPivot += cCntWrt;
    sRest -= cCntWrt;
    if (sRest <= 0)
        EXIT1(CLS_FAIL_PTN_CREATE, EXIT, "Error: %s.", FAIL_PTN_CREATE);

    /* Prepare the indentation. */
    uint8_t ucIndent = cCntWrt;
    char szIndent[BUF_SIZE_INDENT];
    memset(szIndent, 0, sizeof(char) * BUF_SIZE_INDENT);
    uint8_t ucIdx;
    for (ucIdx = 0 ; ucIdx < ucIndent ; ucIdx++)
        szIndent[ucIdx] = ' ';

    /* Print the hex byte block as plaintext string. */
    for (ucIdx = 0 ; ucIdx < p_Conf->ucSizeBlk ; ucIdx++) {
        if (a_usCtn[ucIdx] != WILD_CARD_MARK)
            cCntWrt = snprintf(szPivot, sRest, "%02x ", a_usCtn[ucIdx] & EXTENSION_MASK);
        else
            cCntWrt = snprintf(szPivot, sRest, "?? ");
        szPivot += cCntWrt;
        sRest -= cCntWrt;
        if (sRest <= 0)
            EXIT1(CLS_FAIL_PTN_CREATE, EXIT, "Error: %s.", FAIL_PTN_CREATE);    

        /* Newline if the number of writtern bytes exceeding the line boundary.*/
        if ((ucIdx % HEX_CHUNK_SIZE == HEX_CHUNK_SIZE - 1) && 
            (ucIdx != p_Conf->ucSizeBlk - 1)) {
            cCntWrt = snprintf(szPivot, sRest, "\n%s", szIndent);
            szPivot += cCntWrt;
            sRest -= cCntWrt;
            if (sRest <= 0)
                EXIT1(CLS_FAIL_PTN_CREATE, EXIT, "Error: %s.", FAIL_PTN_CREATE);
        }
    }

    cCntWrt = snprintf(szPivot, sRest, "}\n\n");
    szPivot += cCntWrt;
    sRest -= cCntWrt;
    if (sRest <= 0)
        EXIT1(CLS_FAIL_PTN_CREATE, EXIT, "Error: %s.", FAIL_PTN_CREATE);
    *p_sLen = szPivot - szPtnStr;

EXIT:
    return cRtnCode;
}


int8_t
_PtnPrintConditionSection(char *szPtnCond, GArray *a_CtnAddr, int16_t *p_sLen,
                          uint8_t ucCntBlk, uint8_t ucIdxBlk)
{
    int8_t cRtnCode = CLS_SUCCESS;

    char *szPivot = szPtnCond + *p_sLen;
    int16_t sRest = BUF_SIZE_PTN_SECTION - *p_sLen;
    if (sRest <= 0)
        EXIT1(CLS_FAIL_PTN_CREATE, EXIT, "Error: %s.", FAIL_PTN_CREATE);

    g_array_sort(a_CtnAddr, DsCompContentAddr);
    int8_t cCntWrt;
    uint64_t ulCntAddr = a_CtnAddr->len;
    uint64_t ulIdx;
    CONTENT_ADDR addrPred = {0};
    for (ulIdx = 0 ; ulIdx < ulCntAddr ; ulIdx++) {
        CONTENT_ADDR addrCurr = g_array_index(a_CtnAddr, CONTENT_ADDR, ulIdx);
        if ((addrCurr.iIdSec == addrPred.iIdSec) && 
            (addrCurr.ulOfstRel == addrPred.ulOfstRel))
                continue;

        cCntWrt = snprintf(szPivot, sRest,
                           "%s%s$%s_%d at pe.sections[%d].raw_data_offset + 0x%lx",
                           SPACE_SUBS_TAB, SPACE_SUBS_TAB, PREFIX_HEX_STRING,
                           ucIdxBlk, addrCurr.iIdSec, addrCurr.ulOfstRel);
        szPivot += cCntWrt;
        sRest -= cCntWrt;
        if (sRest <= 0)
            EXIT1(CLS_FAIL_PTN_CREATE, EXIT, "Error: %s.", FAIL_PTN_CREATE);

        if (ulIdx < (ulCntAddr - 1)) {
            cCntWrt = snprintf(szPivot, sRest, " or");
            szPivot += cCntWrt;
            sRest -= cCntWrt;
            if (sRest <= 0)
                EXIT1(CLS_FAIL_PTN_CREATE, EXIT, "Error: %s.", FAIL_PTN_CREATE);
        }

        cCntWrt = snprintf(szPivot, sRest, "\n");
        szPivot += cCntWrt;
        sRest -= cCntWrt;
        if (sRest <= 0)
            EXIT1(CLS_FAIL_PTN_CREATE, EXIT, "Error: %s.", FAIL_PTN_CREATE);

        addrPred = addrCurr;
    }

    if (ucIdxBlk < (ucCntBlk - 1))
        cCntWrt = snprintf(szPivot, sRest, "%s%sor\n", SPACE_SUBS_TAB, SPACE_SUBS_TAB);
    else
        cCntWrt = snprintf(szPivot, sRest, "\n");
    szPivot += cCntWrt;
    sRest -= cCntWrt;
    if (sRest <= 0)
        EXIT1(CLS_FAIL_PTN_CREATE, EXIT, "Error: %s.", FAIL_PTN_CREATE);
    *p_sLen = szPivot - szPtnCond;

EXIT:
    return cRtnCode;
}


int8_t
_PtnSetParamThrdCrt(THREAD_CRAFT *p_Param, GROUP *p_Grp)
{
    p_Param->p_Grp = p_Grp;
    return CLS_SUCCESS;
}


int8_t
_PtnSetParamThrdSlt(THREAD_SLOT *p_Param, GROUP *p_Grp, uint64_t ulIdxBgn, uint64_t ulIdxEnd)
{
    int8_t cRtnCode = CLS_SUCCESS;

    p_Param->ulIdxBgn = ulIdxBgn;
    p_Param->ulIdxEnd = ulIdxEnd;
    p_Param->a_Mbr = p_Grp->a_Mbr;
    
    /* The first slot should be assigned with the block list of this group. */
    if (ulIdxBgn == 0)
        p_Param->a_BlkCand = p_Grp->a_BlkCand;
    else {
        p_Param->a_BlkCand = g_ptr_array_new_with_free_func(DsDeleteBlkCand);
        if (!p_Param->a_BlkCand)
            EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));
    }

    uint64_t ulRange = ulIdxEnd - ulIdxBgn;
    p_Param->a_szBin = (char**)malloc(sizeof(char*) * ulRange);
    if (!p_Param->a_szBin)
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    uint64_t ulOfst;
    for (ulOfst = 0 ; ulOfst < ulRange ; ulOfst++)
        p_Param->a_szBin[ulOfst] = NULL;

    uint64_t ulIdx;
    for (ulIdx = ulIdxBgn, ulOfst = 0 ; ulIdx < ulIdxEnd ; ulIdx++, ulOfst++) {
        uint64_t ulIdSlc = g_array_index(p_Grp->a_Mbr, uint64_t, ulIdx);
        SLICE *p_Slc = g_ptr_array_index(p_Pot->a_Slc, ulIdSlc);

        p_Param->a_szBin[ulOfst] = (char*)malloc(sizeof(char) * p_Slc->usSize);
        if (!(p_Param->a_szBin[ulOfst]))
            EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

        FILE *fp = fopen(p_Slc->szPathFile, "rb");
        if (!fp)
            EXIT1(CLS_FAIL_FILE_IO, EXIT, "Error: %s.", strerror(errno));

        int8_t cStat = fseek(fp, p_Slc->ulOfstAbs, SEEK_SET);
        if (cStat != 0) {
            EXIT1(CLS_FAIL_FILE_IO, CLOSE, "Error: %s.", strerror(errno));
        }

        size_t nReadExpt = p_Slc->usSize;
        size_t nReadReal = fread(p_Param->a_szBin[ulOfst], sizeof(char), nReadExpt, fp);
        if (nReadExpt != nReadReal)
            EXIT1(CLS_FAIL_FILE_IO, CLOSE, "Error: %s.", strerror(errno));
        
        if (p_Slc->usSize < p_Param->usSizeMinSlc)
            p_Param->usSizeMinSlc = p_Slc->usSize;

    CLOSE:
        if (fp)
            fclose(fp);
        if (cRtnCode != CLS_SUCCESS)    
            break;
    }

EXIT:
    return cRtnCode;
}


int8_t
_PtnInitArrayThrdCrt(THREAD_CRAFT **p_aParam, uint64_t ulSize)
{
    int8_t cRtnCode = CLS_SUCCESS;

    *p_aParam = (THREAD_CRAFT*)malloc(sizeof(THREAD_CRAFT) * ulSize);
    if (!(*p_aParam))
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    THREAD_CRAFT *a_Param = *p_aParam;
    uint32_t ulIdx;
    for (ulIdx = 0 ; ulIdx < ulSize ; ulIdx++) {
        a_Param[ulIdx].p_Grp = NULL;
    }

EXIT:
    return cRtnCode;
}


int8_t
_PtnInitArrayThrdSlt(THREAD_SLOT **p_aParam, uint64_t ulSize)
{
    int8_t cRtnCode = CLS_SUCCESS;

    *p_aParam = (THREAD_SLOT*)malloc(sizeof(THREAD_SLOT) * ulSize);
    if (!(*p_aParam))
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    THREAD_SLOT *a_Param = *p_aParam;
    uint64_t ulIdx;
    for (ulIdx = 0 ; ulIdx < ulSize ; ulIdx++) {
        a_Param[ulIdx].usSizeMinSlc = USHRT_MAX;
        a_Param[ulIdx].a_szBin = NULL;
        a_Param[ulIdx].a_Mbr = NULL;
        a_Param[ulIdx].a_BlkCand = NULL;
    }

EXIT:
    return cRtnCode;
}


int8_t
_PtnDeinitArrayThrdCrt(THREAD_CRAFT *a_Param, uint64_t ulSize)
{
    if (a_Param)
        free(a_Param);
    
    return CLS_SUCCESS;
}


int8_t
_PtnDeinitArrayThrdSlt(THREAD_SLOT *a_Param, uint64_t ulSize)
{
    if (!a_Param)
        return CLS_SUCCESS;

    uint64_t ulIdx;
    for (ulIdx = 0 ; ulIdx < ulSize ; ulIdx++) {
        THREAD_SLOT *p_Param = a_Param + ulIdx;
        if (p_Param->a_BlkCand) {
            /* We now do not free the first array which represents as the
               common features shared by this group. */
            if (ulIdx > 0)
                g_ptr_array_free(p_Param->a_BlkCand, true);
        }
        if (p_Param->a_szBin) {
            uint64_t ulRange = p_Param->ulIdxEnd - p_Param->ulIdxBgn;
            uint64_t ulOfst;
            for (ulOfst = 0 ; ulOfst < ulRange ; ulOfst++) {
                if (p_Param->a_szBin[ulOfst])
                    free(p_Param->a_szBin[ulOfst]);
            }
            free(p_Param->a_szBin);
        }
    }

    free(a_Param);
    return CLS_SUCCESS;
}
