#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include "spew.h"
#include "data.h"
#include "cluster.h"
#include "correlate.h"
#include "slice.h"
#include "similarity.h"


sem_t ins_Sem;
CONFIG *p_Conf;
MELT_POT *p_Pot;
PLUGIN_SLICE *plg_Slc;
PLUGIN_SIMILARITY *plg_Sim;


/*======================================================================*
 *                 Declaration for Internal Functions                   *
 *======================================================================*/
/**
 * This function slices a file and generates the hash for each slice.
 * 
 * @param vp_Param      The pointer to the structure which should be
 *                      recorded with the slicing result.
 * 
 * @return (currently deprecated)
 */
void*
_CrlMapSlice(void *vp_Param);


/**
 * This function compares the slices within the designated range and records
 * the slice pair with similarity score fitting the threshold.
 * 
 * @param vp_Param      The pointer to the structure which should be
 *                      recorded with the slice pair.
 * 
 * @return (currently deprecated)
 */
void*
_CrlMapCompare(void *vp_Param);


/**
 * This function iteratively collects the slices and hashes acqured by each
 * thread into MELT_POT structure.
 * 
 * @param p_Param       The pointer to the structure which records the slicing result
 *                      acquired by a thread.
 * @param p_ulIdSlc     The pointer to the variable which should be updated with
 *                      the currently traced index of file slice.
 * 
 * @return (currently unused)
 */
int8_t
_CrlReduceSlice(THREAD_SLICE *p_Param, uint64_t *p_ulIdSlc);


/**
 * This function iteratively merges the similar slice pairs. Note that the 
 * group id is initially assigned to a slice which will be formally updated later.
 * 
 * @param p_Param       The pointer to the structure which records the slice pairs.
 */
int8_t
_CrlReduceCompare(THREAD_COMPARE *p_Param);


/**
 * This function initializes the THREAD_SLICE structure.
 * 
 * @param p_Param       The pointer to the to be initialized structure.
 * @param szName        The sample name.
 * 
 * @return status code
 */
int8_t
_CrlSetParamThrdSlc(THREAD_SLICE *p_Param, char *szName);


/**
 * This function initializes the THREAD_COMPARE structure.
 * 
 * @param p_Param       The pointer to the to be initialized structure.
 * @param ucIdThrd      The index of the thread group.
 * 
 * @return (currently unused)
 */
int8_t
_CrlSetParamThrdCmp(THREAD_COMPARE *p_Param, uint8_t ucIdThrd);


/**
 * This function initializes the array of THREAD_SLICE structures.
 * 
 * @param p_aParam      The pointer to the array of THREAD_SLICE structures.
 * @param uiSize        The array size.
 * 
 * @return status code
 */
int8_t
_CrlInitArrayThrdSlc(THREAD_SLICE **p_aParam, uint32_t uiSize);


/**
 * This function initializes the array of THREAD_COMPARE structures.
 * 
 * @param p_aParam      The pointer to the array of THREAD_COMPARE structures.
 * @param uiSize        The array size.
 * 
 * @return status code
 */
int8_t
_CrlInitArrayThrdCmp(THREAD_COMPARE **p_aParam, uint32_t uiSize);


/**
 * This function deinitializes the array of THREAD_SLICE structures.
 * 
 * @param a_Param       The array of THREAD_SLICE structures.
 * @param uiSize        The array size.
 * @param bClean        The flag which hints for early clean of the slicing result.
 * 
 * @return (currently unused)
 */
int8_t
_CrlDeinitArrayThrdSlc(THREAD_SLICE *a_Param, uint32_t uiSize, bool bClean);


/**
 * This function deinitializes the array of THREAD_COMPARE structures.
 * 
 * @param a_Param       The array of THREAD_COMPARE structures.
 * @param uiSize        The array size.
 * 
 * @return (currently unused)
 */
int8_t
_CrlDeinitArrayThrdCmp(THREAD_COMPARE *a_Param, uint32_t uiSize);


/*======================================================================*
 *                Implementation for Exported Functions                 *
 *======================================================================*/
int8_t
CrlSetContext(CONTEXT *p_Ctx)
{
    p_Conf = p_Ctx->p_Conf;
    p_Pot = p_Ctx->p_Pot;
    plg_Slc = p_Ctx->plg_Slc;
    plg_Sim = p_Ctx->plg_Sim;

    return CLS_SUCCESS;
}


int8_t
CrlPrepareSlice()
{
    int8_t cRtnCode = CLS_SUCCESS;

    DIR *dirRoot = opendir(p_Conf->szPathRootIn);
    if (!dirRoot)
        EXIT1(CLS_FAIL_FILE_IO, EXIT, "Error: %s.", strerror(errno));

    /* Record the filenames and count the number. */
    p_Pot->a_Name = g_ptr_array_new_with_free_func(DsFreeNameArray);
    if (!p_Pot->a_Name)
        EXIT1(CLS_FAIL_MEM_ALLOC, CLOSEDIR, "Error: %s.", strerror(errno));

    uint32_t uiCntFile = 0;
    struct dirent *entFile;
    while (entFile = readdir(dirRoot)) {
        if (!strcmp(entFile->d_name, ".") || !strcmp(entFile->d_name, ".."))
            continue;

        uiCntFile++;
        g_ptr_array_add(p_Pot->a_Name, (gpointer)strdup(entFile->d_name));
    }
    if (uiCntFile == 0)
        EXIT1(CLS_FAIL_FILE_IO, CLOSEDIR, "Error: %s.", FAIL_NO_SAMPLE);

    /*-----------------------------------------------------------------------*
     * Spawn multipe threads each of which:                                  *
     * 1. Slices a given file and returns an array of SLICE structure.       *
     * 2. Generate an array of hashes which are derived from file slices.    *
     *-----------------------------------------------------------------------*/
    THREAD_SLICE *a_Param;
    int8_t cStat = _CrlInitArrayThrdSlc(&a_Param, uiCntFile);
    if (cStat != CLS_SUCCESS)
        EXITQ(CLS_FAIL_MEM_ALLOC, CLOSEDIR);

    sem_init(&ins_Sem, 0, p_Conf->ucCntThrd);
    uint32_t uiIdx = 0;
    while (uiIdx < uiCntFile) {
        sem_wait(&ins_Sem);
        _CrlSetParamThrdSlc(&(a_Param[uiIdx]), g_ptr_array_index(p_Pot->a_Name, uiIdx));
        pthread_create(&(a_Param[uiIdx].tId), NULL, _CrlMapSlice, (void*)&(a_Param[uiIdx]));
        uiIdx++;                    
    }

    /*-----------------------------------------------------------------------*
     * Join the spawned threads:                                             *
     * 1. Merge the array of SLICE structures into MELT_POT structure.       *
     * 2. Merge the array of hashes into MELT_POT structure.                 *
     *-----------------------------------------------------------------------*/
    bool bClean = false;
    p_Pot->a_Slc = g_ptr_array_new_with_free_func(plg_Slc->FreeSliceArray);
    if (!p_Pot->a_Slc) {
        bClean = true;
        EXIT1(CLS_FAIL_MEM_ALLOC, FREEPARAM, "Error: %s.", strerror(errno));
    }
    p_Pot->a_Hash = g_ptr_array_new_with_free_func(DsFreeHashArray);
    if (!p_Pot->a_Hash) {
        bClean = true;
        EXIT1(CLS_FAIL_MEM_ALLOC, FREEPARAM, "Error: %s.", strerror(errno));
    }
    uint64_t ulIdSlc = 0;
    for (uiIdx = 0 ; uiIdx < uiCntFile ; uiIdx++) {
        pthread_join(a_Param[uiIdx].tId, NULL);
        _CrlReduceSlice(&(a_Param[uiIdx]), &ulIdSlc);
        if (a_Param[uiIdx].cRtnCode != CLS_SUCCESS)
            cRtnCode = CLS_FAIL_PROCESS;
    }
    sem_destroy(&ins_Sem);

FREEPARAM:
    _CrlDeinitArrayThrdSlc(a_Param, uiCntFile, bClean);
CLOSEDIR:
    if (dirRoot)
        closedir(dirRoot);
EXIT:
    return cRtnCode;
}


int8_t
CrlCorrelateSlice()
{
    int8_t cRtnCode = CLS_SUCCESS;

    /*-----------------------------------------------------------------------*
     * Spawn multipe threads each of which:                                  *
     * 1. Do pairwise similarity computation using the given range of hashes.*
     * 2. Record the slice pair with similarity score fitting the threshold. *
     *-----------------------------------------------------------------------*/
    THREAD_COMPARE *a_Param;
    int8_t cStat = _CrlInitArrayThrdCmp(&a_Param, p_Conf->ucCntThrd);
    if (cStat != CLS_SUCCESS)
        EXITQ(CLS_FAIL_MEM_ALLOC, EXIT);

    uint8_t ucIdx;
    for (ucIdx = 0 ; ucIdx < p_Conf->ucCntThrd ; ucIdx++) {
        _CrlSetParamThrdCmp(&(a_Param[ucIdx]), ucIdx + 1);
        pthread_create(&(a_Param[ucIdx].tId), NULL, _CrlMapCompare, (void*)&(a_Param[ucIdx]));
    }

    /*-----------------------------------------------------------------------*
     * Join the spawned threads:                                             *
     * 1. Merge the slice pairs and assign an initial group id to each SLICE * 
     *    structure. The group id will be updated upon group construction.   *
     *-----------------------------------------------------------------------*/
    for (ucIdx = 0 ; ucIdx < p_Conf->ucCntThrd ; ucIdx++) {
        pthread_join(a_Param[ucIdx].tId, NULL);
        _CrlReduceCompare(&(a_Param[ucIdx]));
        if (a_Param[ucIdx].cRtnCode != CLS_SUCCESS)
            cRtnCode = CLS_FAIL_PROCESS;
    }

FREEPARAM:    
    _CrlDeinitArrayThrdCmp(a_Param, p_Conf->ucCntThrd);    
EXIT:
    return cRtnCode;
}


/*======================================================================*
 *                Implementation for Internal Functions                 *
 *======================================================================*/
void*
_CrlMapSlice(void *vp_Param)
{
    int8_t cRtnCode = CLS_SUCCESS;
    THREAD_SLICE *p_Param = (THREAD_SLICE*)vp_Param;

    /* Extract the file slices. */
    int8_t cStat = plg_Slc->GetFileSlice(p_Param->szPath, p_Conf->usSizeSlc, 
                                         &(p_Param->a_Slc));
    if (cStat != SLC_SUCCESS)
        EXITQ(CLS_FAIL_PLUGIN_INTERACT, EXIT);

    /* Generate the hashes for all file slices. */
    FILE *fp = fopen(p_Param->szPath, "rb");
    if (!fp)
        EXIT1(CLS_FAIL_FILE_IO, EXIT, "Error: %s.", strerror(errno));
    
    char *szBin = (char*)malloc(sizeof(char) * p_Conf->usSizeSlc);
    if (!szBin)
        EXIT1(CLS_FAIL_MEM_ALLOC, CLOSEFILE, "Error: %s.", strerror(errno));

    p_Param->a_Hash = g_ptr_array_new();
    if (!p_Param->a_Hash)
        EXIT1(CLS_FAIL_MEM_ALLOC, FREEBIN, "Error: %s.", strerror(errno));
    
    uint64_t ulCntSlc = p_Param->a_Slc->len;
    uint64_t ulIdx;
    for (ulIdx = 0 ; ulIdx < ulCntSlc ; ulIdx++) {
        SLICE *p_Slc = g_ptr_array_index(p_Param->a_Slc, ulIdx);
        cStat = fseek(fp, p_Slc->ulOfstAbs, SEEK_SET);
        if (cStat != 0)
            EXIT1(CLS_FAIL_FILE_IO, FREEBIN, "Error: %s.", strerror(errno));

        size_t nReadExpt = p_Slc->usSize;
        size_t nReadReal = fread(szBin, sizeof(char), nReadExpt, fp);
        if (nReadExpt != nReadReal)
            EXIT1(CLS_FAIL_FILE_IO, FREEBIN, "Error: %s.", strerror(errno));

        char *szHash;
        cStat = plg_Sim->GetHash(szBin, p_Slc->usSize, &szHash, NULL);
        if (cStat != SLC_SUCCESS)
            EXITQ(CLS_FAIL_PLUGIN_INTERACT, EXIT);
        g_ptr_array_add(p_Param->a_Hash, (gpointer)szHash);
    }

FREEBIN:
    if (szBin)
        free(szBin);
CLOSEFILE:
    if (fp)
        fclose(fp);
EXIT:
    p_Param->cRtnCode = cRtnCode;
    sem_post(&ins_Sem);
    return;
}


void*
_CrlMapCompare(void *vp_Param)
{
    int8_t cRtnCode = CLS_SUCCESS;
    THREAD_COMPARE *p_Param = (THREAD_COMPARE*)vp_Param;
    uint8_t ucIdThrd = p_Param->ucIdThrd;
    uint8_t ucCntThrd = p_Conf->ucCntThrd;
    uint64_t ulCntSlc = p_Pot->a_Hash->len;

    p_Param->a_Bind = g_ptr_array_new_with_free_func(DsFreeBindArray);
    if (!p_Param->a_Bind)
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    /* The thread index should start from "1". */
    uint64_t ulIdSrc = 0;
    uint64_t ulIdTge = ulIdSrc + ucIdThrd - ucCntThrd;
    while (true) {
        ulIdTge += ucCntThrd;
        while (ulIdTge >= ulCntSlc) {
            ulIdSrc++;
            if (ulIdSrc == ulCntSlc)
                break;
            ulIdTge -= ulCntSlc;
            ulIdTge += ulIdSrc + 1;    
        }
        if ((ulIdSrc >= ulCntSlc) || (ulIdTge >= ulCntSlc))
            break;

        char *szSrc = g_ptr_array_index(p_Pot->a_Hash, ulIdSrc);
        char *szTge = g_ptr_array_index(p_Pot->a_Hash, ulIdTge);
        uint32_t uiLenSrc = strlen(szSrc);
        uint32_t uiLenTge = strlen(szTge);
        uint8_t ucSim;
        int8_t cStat = plg_Sim->CompareHashPair(szSrc, uiLenSrc,
                                                szTge, uiLenTge, &ucSim);
        if (cStat != SIM_SUCCESS)
            EXITQ(CLS_FAIL_PLUGIN_INTERACT, EXIT);

        if (ucSim >= p_Conf->ucScoreSim) {
            BIND *p_Bind = (BIND*)malloc(sizeof(BIND));
            p_Bind->ulIdSlcSrc = ulIdSrc;
            p_Bind->ulIdSlcTge = ulIdTge;
            g_ptr_array_add(p_Param->a_Bind, (gpointer)p_Bind);
        }
    }

EXIT:
    p_Param->cRtnCode = cRtnCode;
    return;
}


int8_t
_CrlReduceSlice(THREAD_SLICE *p_Param, uint64_t *p_ulIdSlc)
{
    uint64_t ulIdBase = *p_ulIdSlc;
    uint64_t ulLen = p_Param->a_Slc->len;
    uint64_t ulIdx;
    GPtrArray *a_Slc = p_Param->a_Slc;
    GPtrArray *a_Hash = p_Param->a_Hash;

    for (ulIdx = 0 ; ulIdx < ulLen ; ulIdx++) {
        SLICE *p_Slc = g_ptr_array_index(a_Slc, ulIdx);
        p_Slc->ulIdSlc = ulIdBase + ulIdx;
        g_ptr_array_add(p_Pot->a_Slc, (gpointer)p_Slc);

        char *szHash = g_ptr_array_index(a_Hash, ulIdx);
        g_ptr_array_add(p_Pot->a_Hash, (gpointer)szHash);
    }

    *p_ulIdSlc += ulLen;
    return CLS_SUCCESS;
}


int8_t
_CrlReduceCompare(THREAD_COMPARE *p_Param)
{
    uint64_t ulLen = p_Param->a_Bind->len;
    uint64_t ulIdx;

    for (ulIdx = 0 ; ulIdx < ulLen ; ulIdx++) {
        BIND *p_Bind = g_ptr_array_index(p_Param->a_Bind, ulIdx);
        uint64_t ulIdSrc = p_Bind->ulIdSlcSrc;
        uint64_t ulIdTge = p_Bind->ulIdSlcTge;
        uint64_t ulIdGrp = MIN(ulIdSrc, ulIdTge);

        SLICE *p_SlcSrc = g_ptr_array_index(p_Pot->a_Slc, ulIdSrc);
        SLICE *p_SlcTge = g_ptr_array_index(p_Pot->a_Slc, ulIdTge);
        p_SlcSrc->ulIdGrp = ulIdGrp;
        p_SlcTge->ulIdGrp = ulIdGrp;
    }

    return CLS_SUCCESS;
}


int8_t
_CrlSetParamThrdSlc(THREAD_SLICE *p_Param, char *szName)
{
    int8_t cRtnCode = CLS_SUCCESS;    

    int32_t iLen = strlen(p_Conf->szPathRootIn) + strlen(szName) + 2;
    p_Param->szPath = (char*)malloc(sizeof(char) * iLen);
    if (!p_Param->szPath)
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    snprintf(p_Param->szPath, iLen, "%s/%s", p_Conf->szPathRootIn, szName);

EXIT:    
    return cRtnCode;
}


int8_t
_CrlSetParamThrdCmp(THREAD_COMPARE *p_Param, uint8_t ucIdThrd)
{
    p_Param->ucIdThrd = ucIdThrd;
    p_Param->a_Bind = NULL;
    return CLS_SUCCESS;
}


int8_t
_CrlInitArrayThrdSlc(THREAD_SLICE **p_aParam, uint32_t uiSize)
{
    int8_t cRtnCode = CLS_SUCCESS;

    *p_aParam = (THREAD_SLICE*)malloc(sizeof(THREAD_SLICE) * uiSize);
    if (!(*p_aParam))
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    THREAD_SLICE *a_Param = *p_aParam;
    uint32_t uiIdx;
    for (uiIdx = 0 ; uiIdx < uiSize ; uiIdx++) {
        a_Param[uiIdx].a_Slc = NULL;
        a_Param[uiIdx].a_Hash = NULL;
        a_Param[uiIdx].szPath = NULL;
    }

EXIT:
    return cRtnCode;
}


int8_t
_CrlInitArrayThrdCmp(THREAD_COMPARE **p_aParam, uint32_t uiSize)
{
    int8_t cRtnCode = CLS_SUCCESS;

    *p_aParam = (THREAD_COMPARE*)malloc(sizeof(THREAD_COMPARE) * uiSize);
    if (!(*p_aParam))
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    THREAD_COMPARE *a_Param = *p_aParam;
    uint32_t uiIdx;
    for (uiIdx = 0 ; uiIdx < uiSize ; uiIdx++)
        a_Param[uiIdx].a_Bind = NULL;

EXIT:
    return cRtnCode;
}


int8_t
_CrlDeinitArrayThrdSlc(THREAD_SLICE *a_Param, uint32_t uiSize, bool bClean)
{
    if (!a_Param)
        return CLS_SUCCESS;

    uint32_t uiIdx;
    for (uiIdx = 0 ; uiIdx < uiSize ; uiIdx++) {
        if (bClean) {
            g_ptr_array_set_free_func(a_Param[uiIdx].a_Hash, DsFreeHashArray);
            g_ptr_array_set_free_func(a_Param[uiIdx].a_Slc, plg_Slc->FreeSliceArray);
        }
        if (a_Param[uiIdx].a_Slc)
            g_ptr_array_free(a_Param[uiIdx].a_Slc, true);

        if (a_Param[uiIdx].a_Hash)
            g_ptr_array_free(a_Param[uiIdx].a_Hash, true);

        if (a_Param[uiIdx].szPath)
            free(a_Param[uiIdx].szPath);
    }
    
    free(a_Param);
    return CLS_SUCCESS;
}


int8_t
_CrlDeinitArrayThrdCmp(THREAD_COMPARE *a_Param, uint32_t uiSize)
{
    if (!a_Param)
        return CLS_SUCCESS;

    uint32_t uiIdx;
    for (uiIdx = 0 ; uiIdx < uiSize ; uiIdx++) {
        if (a_Param[uiIdx].a_Bind)
            g_ptr_array_free(a_Param[uiIdx].a_Bind, true);
    }

    free(a_Param);
    return CLS_SUCCESS;        
}
