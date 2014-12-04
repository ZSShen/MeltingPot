#include <stdio.h>
#include <stdlib.h>
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


/**
 * This function slices a file and generates the hash for each slice.
 * 
 * @param vp_Param      The pointer to the parameter which should be
 *                      recorded with slicing result.
 * 
 * @return (currently deprecated)
 */
void*
_CrlHandleSlice(void *vp_Param);


/**
 * This function initializes the THREAD_SLICE structure.
 * 
 * @param p_Param       The pointer to the to be initialized structure.
 * @param p_Conf        The pointer to the user specified configuration.
 * @param plg_Slc       The handle to the file slicing plugin.
 * @param szName    The sample name.
 * 
 * @return status code
 */
int8_t
_CrlSetParamThrdSlc(THREAD_SLICE *p_Param, CONFIG *p_Conf, PLUGIN_SLICE *plg_Slc,
                    char *szName);


/**
 * This function deinitializes the THREAD_SLICE structure.
 * 
 * @param p_Param       The pointer to the to be deinitialized structure.
 */
int8_t
_CrlFreeParamThrdSlc(THREAD_SLICE *p_Param);


int8_t
CrlPrepareSlice(MELT_POT *p_Pot, CONFIG *p_Conf, PLUGIN_SLICE *plg_Slc)
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

    /* Spwan multipe threads each of which slices a file and returns an array of
       SLICE structure and the corresponding array of hashes. */
    THREAD_SLICE *a_Param = (THREAD_SLICE*)malloc(sizeof(THREAD_SLICE) * iCntFile);
    if (!a_Param) {
        EXIT1(CLS_FAIL_MEM_ALLOC, CLOSEDIR, "Error: %s.", strerror(errno));
    }
    sem_init(&ins_Sem, 0, p_Conf->ucCntThrd);
    int32_t iIdx = 0;
    while (iIdx < iCntFile) {
        sem_wait(&ins_Sem);
        char *szName = g_ptr_array_index(p_Pot->a_Name, iIdx);
        _CrlSetParamThrdSlc(&(a_Param[iIdx]), p_Conf, plg_Slc, szName);
        pthread_create(&(a_Param[iIdx].tId), NULL, _CrlHandleSlice, (void*)&(a_Param[iIdx]));
        iIdx++;                    
    }

    /* Reduce the results and put them into MELT_POT structure. */
    for (iIdx = 0 ; iIdx < iCntFile ; iIdx++) {
        pthread_join(a_Param[iIdx].tId, NULL);
    }
    sem_destroy(&ins_Sem);

FREEPARAM:
    if (a_Param) {
        for (iIdx = 0 ; iIdx < iCntFile ; iIdx++) {
            _CrlFreeParamThrdSlc(&(a_Param[iIdx]));
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


void* _CrlHandleSlice(void *vp_ThrdParam)
{
    /*
    int8_t cRtnCode = CLS_SUCCESS;
    THREAD_SLICE *p_Param = (THREAD_SLICE*)vp_ThrdParam;

    char szPath[PATH_BUF_SIZE];
    snprintf(szPath, PATH_BUF_SIZE, "%s/%s", p_Param->p_Conf->szPathRootIn,
                                                 p_Param->szName);
    int8_t cStat = p_Param->plg_Slc->GetFileSlice(szPath, p_Param->p_Conf->usSizeSlc, 
                                                  &(p_Param->a_Slc));
    if (cStat != SLC_SUCCESS) {
        EXITQ(CLS_FAIL_PLUGIN_INTERACT, EXIT);
    }
    */
    sem_post(&ins_Sem);

EXIT:
    return;
}


int8_t
_CrlSetParamThrdSlc(THREAD_SLICE *p_Param, CONFIG *p_Conf, PLUGIN_SLICE *plg_Slc, 
                    char *szName)
{
    int8_t cRtnCode = CLS_SUCCESS;    

    p_Param->a_Slc = NULL;
    p_Param->a_Hash = NULL;
    p_Param->plg_Slc = plg_Slc;
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
_CrlFreeParamThrdSlc(THREAD_SLICE *p_Param)
{
    if (p_Param->a_Slc) {
        g_ptr_array_free(p_Param->a_Slc, TRUE);
    }
    if (p_Param->a_Hash) {
        g_ptr_array_free(p_Param->a_Hash, TRUE);
    }
    if (p_Param->szPath) {
        free(p_Param->szPath);
    }

    return CLS_SUCCESS;
}
