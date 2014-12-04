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
 * This function iteratively collects the slices and hashes acqured by each
 * thread into MELT_POT structure.
 * 
 * @param p_Pot         The pointer to the structure which records the clustering progress.
 * @param p_Param       The pointer to the structure which records the slicing result
 *                      acquired by a thread.
 * @param p_ulIdSlc     The pointer to the variable which should be updated with
 *                      the currently traced index of file slice.
 * 
 * @return (currently unused)
 */
int8_t
_CrlReduceSlice(MELT_POT *p_Pot, THREAD_SLICE *p_Param, uint64_t *p_ulIdSlc);


/**
 * This function initializes the THREAD_SLICE structure.
 * 
 * @param p_Param       The pointer to the to be initialized structure.
 * @param p_Conf        The pointer to the user specified configuration.
 * @param szName        The sample name.
 * @param plg_Slc       The handle to the file slicing plugin.
 * @param plg_Sim       The handle to the similarity computatoin plugin.
 * 
 * @return status code
 */
int8_t
_CrlSetParamThrdSlc(THREAD_SLICE *p_Param, CONFIG *p_Conf, char *szName,
                    PLUGIN_SLICE *plg_Slc, PLUGIN_SIMILARITY *plg_Sim);


/**
 * This function deinitializes the THREAD_SLICE structure.
 * 
 * @param p_Param       The pointer to the to be deinitialized structure.
 * @param bClean        The flag which hints for early clean of the slicing result.
 * @param plg_Slc       The handle to the file slicing plugin.
 * 
 * @return (currently unused)
 */
int8_t
_CrlFreeParamThrdSlc(THREAD_SLICE *p_Param, bool bClean, PLUGIN_SLICE *plg_Slc);


/*======================================================================*
 *                Implementation for Exported Functions                 *
 *======================================================================*/
int8_t
CrlPrepareSlice(MELT_POT *p_Pot, CONFIG *p_Conf, PLUGIN_SLICE *plg_Slc,
                PLUGIN_SIMILARITY *plg_Sim)
{
    int8_t cRtnCode = CLS_SUCCESS;

    DIR *dirRoot = opendir(p_Conf->szPathRootIn);
    if (!dirRoot) {
        EXIT1(CLS_FAIL_FILE_IO, EXIT, "Error: %s.", strerror(errno));
    }

    /* Record the filenames and count the number. */
    p_Pot->a_Name = g_ptr_array_new_with_free_func(DsFreeNameArray);
    if (!p_Pot->a_Name) {
        EXIT1(CLS_FAIL_MEM_ALLOC, CLOSEDIR, "Error: %s.", strerror(errno));
    }
    uint32_t iCntFile = 0;
    struct dirent *entFile;
    while (entFile = readdir(dirRoot)) {
        if ((strcmp(entFile->d_name, ".") == 0) ||
            (strcmp(entFile->d_name, "..") == 0)) {
            continue;
        }
        iCntFile++;
        g_ptr_array_add(p_Pot->a_Name, (gpointer)strdup(entFile->d_name));
    }
    if (iCntFile == 0) {
        EXIT1(CLS_FAIL_FILE_IO, CLOSEDIR, "Error: %s.", FAIL_NO_SAMPLE);
    }

    /* ----------------------------------------------------------------------*
     * Spawn multipe threads each of which:                                  *
     * 1. Slices a given file and returns an array of SLICE structure.       *
     * 2. Generate an array of hashes each of which is derived a file slice. *
     *-----------------------------------------------------------------------*/
    THREAD_SLICE *a_Param = (THREAD_SLICE*)malloc(sizeof(THREAD_SLICE) * iCntFile);
    if (!a_Param) {
        EXIT1(CLS_FAIL_MEM_ALLOC, CLOSEDIR, "Error: %s.", strerror(errno));
    }
    sem_init(&ins_Sem, 0, p_Conf->ucCntThrd);
    int32_t iIdx = 0;
    while (iIdx < iCntFile) {
        sem_wait(&ins_Sem);
        char *szName = g_ptr_array_index(p_Pot->a_Name, iIdx);
        _CrlSetParamThrdSlc(&(a_Param[iIdx]), p_Conf, szName, plg_Slc, plg_Sim);
        pthread_create(&(a_Param[iIdx].tId), NULL, _CrlMapSlice, (void*)&(a_Param[iIdx]));
        iIdx++;                    
    }

    /* ----------------------------------------------------------------------*
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
    for (iIdx = 0 ; iIdx < iCntFile ; iIdx++) {
        pthread_join(a_Param[iIdx].tId, NULL);
        _CrlReduceSlice(p_Pot, &(a_Param[iIdx]), &ulIdSlc);
    }
    sem_destroy(&ins_Sem);

FREEPARAM:
    if (a_Param) {
        for (iIdx = 0 ; iIdx < iCntFile ; iIdx++) {
            _CrlFreeParamThrdSlc(&(a_Param[iIdx]), bClean, plg_Slc);
        }
        free(a_Param);
    }

CLOSEDIR:
    if (dirRoot) {
        closedir(dirRoot);
    }

EXIT:
    return cRtnCode;
}


int8_t
CrlCorrelateSlice(MELT_POT *p_Pot, CONFIG *p_Conf, PLUGIN_SIMILARITY *plg_Sim)
{
    int8_t cRtnCode = CLS_SUCCESS;
    
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
    PLUGIN_SLICE *plg_Slc = p_Param->plg_Slc;
    int8_t cStat = plg_Slc->GetFileSlice(p_Param->szPath, p_Param->usSizeSlc, 
                                         &(p_Param->a_Slc));
    if (cStat != SLC_SUCCESS) {
        EXITQ(CLS_FAIL_PLUGIN_INTERACT, EXIT);
    }

    /* Generate the hashes for all file slices. */
    FILE *fp = fopen(p_Param->szPath, "rb");
    if (!fp) {
        EXIT1(CLS_FAIL_FILE_IO, EXIT, "Error: %s.", strerror(errno));
    }
    
    PLUGIN_SIMILARITY *plg_Sim = p_Param->plg_Sim;
    char *szBin = (char*)malloc(sizeof(char) * p_Param->usSizeSlc);
    if (!szBin) {
        EXIT1(CLS_FAIL_MEM_ALLOC, CLOSEFILE, "Error: %s.", strerror(errno));
    }
    p_Param->a_Hash = g_ptr_array_new();
    if (!p_Param->a_Hash) {
        EXIT1(CLS_FAIL_MEM_ALLOC, FREEBIN, "Error: %s.", strerror(errno));
    }
    
    uint32_t uiCntSlc = p_Param->a_Slc->len;
    uint32_t uiIdx;
    for (uiIdx = 0 ; uiIdx < uiCntSlc ; uiIdx++) {
        SLICE *p_Slc = g_ptr_array_index(p_Param->a_Slc, uiIdx);
        cStat = fseek(fp, p_Slc->ulOfstAbs, SEEK_SET);
        if (cStat != 0) {
            EXIT1(CLS_FAIL_FILE_IO, FREEBIN, "Error: %s.", strerror(errno));
        }
        size_t nReadExpt = p_Slc->usSize;
        size_t nReadReal = fread(szBin, sizeof(char), nReadExpt, fp);
        if (nReadExpt != nReadReal) {
            EXIT1(CLS_FAIL_FILE_IO, FREEBIN, "Error: %s.", strerror(errno));
        }
        char *szHash;
        cStat = plg_Sim->GetHash(szBin, p_Slc->usSize, &szHash, NULL);
        g_ptr_array_add(p_Param->a_Hash, (gpointer)szHash);
    }

FREEBIN:
    if (szBin) {
        free(szBin);
    }

CLOSEFILE:
    if (fp) {
        fclose(fp);
    }

EXIT:
    sem_post(&ins_Sem);
    return;
}


int8_t
_CrlReduceSlice(MELT_POT *p_Pot, THREAD_SLICE *p_Param, uint64_t *p_ulIdSlc)
{
    uint64_t ulIdBase = *p_ulIdSlc;
    uint32_t uiLen = p_Param->a_Slc->len;
    uint32_t uiIdx;
    GPtrArray *a_Slc = p_Param->a_Slc;
    GPtrArray *a_Hash = p_Param->a_Hash;

    for (uiIdx = 0 ; uiIdx < uiLen ; uiIdx++) {
        SLICE *p_Slc = g_ptr_array_index(a_Slc, uiIdx);
        p_Slc->ulIdSlc = ulIdBase + uiIdx;
        g_ptr_array_add(p_Pot->a_Slc, (gpointer)p_Slc);
        
        char *szHash = g_ptr_array_index(a_Hash, uiIdx);
        g_ptr_array_add(p_Pot->a_Hash, (gpointer)szHash);
    }
    *p_ulIdSlc += uiLen;

    return CLS_SUCCESS;
}


int8_t
_CrlSetParamThrdSlc(THREAD_SLICE *p_Param, CONFIG *p_Conf, char *szName,
                    PLUGIN_SLICE *plg_Slc, PLUGIN_SIMILARITY *plg_Sim)
{
    int8_t cRtnCode = CLS_SUCCESS;    

    p_Param->a_Slc = NULL;
    p_Param->a_Hash = NULL;
    p_Param->plg_Slc = plg_Slc;
    p_Param->plg_Sim = plg_Sim;
    p_Param->usSizeSlc = p_Conf->usSizeSlc;

    int32_t iLen = strlen(p_Conf->szPathRootIn) + strlen(szName) + 2;
    p_Param->szPath = (char*)malloc(sizeof(char) * iLen);
    if (!p_Param->szPath) {
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));
    }
    snprintf(p_Param->szPath, iLen, "%s/%s", p_Conf->szPathRootIn, szName);

EXIT:    
    return cRtnCode;
}


int8_t
_CrlFreeParamThrdSlc(THREAD_SLICE *p_Param, bool bClean, PLUGIN_SLICE *plg_Slc)
{
    if (bClean) {
        g_ptr_array_set_free_func(p_Param->a_Hash, DsFreeHashArray);
        g_ptr_array_set_free_func(p_Param->a_Slc, plg_Slc->FreeSliceArray);
    }

    if (p_Param->a_Slc) {
        g_ptr_array_free(p_Param->a_Slc, true);
    }
    if (p_Param->a_Hash) {
        g_ptr_array_free(p_Param->a_Hash, true);
    }
    if (p_Param->szPath) {
        free(p_Param->szPath);
    }

    return CLS_SUCCESS;
}
