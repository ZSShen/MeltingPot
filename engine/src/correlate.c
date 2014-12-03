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
 * @param vp_ThrdParam     The pointer to the parameter which should be
 *                         recorded with slicing result.
 * 
 * @return (currently deprecated)
 */
void* _CrlHandleSlice(void *vp_ThrdParam);


int8_t
CrlPrepareSlice(MELT_POT *p_Pot, CONFIG *p_Conf, PLUGIN_SLICE *plg_Slc)
{
    int8_t cRtnCode = CLS_SUCCESS;

    DIR *dirRoot = opendir(p_Conf->szPathRootIn);
    if (dirRoot == NULL) {
        EXIT1(CLS_FAIL_FILE_IO, EXIT, "Error: %s.", strerror(errno));
    }

    /* Count the number of samples. */
    uint32_t iCntFile = 0;
    struct dirent *entFile;
    while ((entFile = readdir(dirRoot)) != NULL) {
        if ((strcmp(entFile->d_name, ".") == 0) ||
            (strcmp(entFile->d_name, "..") == 0)) {
            continue;
        }
        iCntFile++;
    }
    if (iCntFile == 0) {
        EXIT1(CLS_FAIL_FILE_IO, CLOSEDIR, "Error: %s.", FAIL_NO_SAMPLE);
    }

    /* Spew multipe threads each of which slices a file and returns an array of
       SLICE structure and the corresponding array of hashes. */
    sem_init(&ins_Sem, 0, p_Conf->ucCntThrd);
    THREAD_SLICE *p_Rec = (THREAD_SLICE*)malloc(sizeof(THREAD_SLICE) * iCntFile);
    int32_t iIdxFile = 0;
    seekdir(dirRoot, iIdxFile);
    while (iIdxFile < iCntFile) {
        sem_wait(&ins_Sem);
        do {
            entFile = readdir(dirRoot);
        } while((strcmp(entFile->d_name, ".") == 0) || 
                (strcmp(entFile->d_name, "..") == 0));
        p_Rec[iIdxFile].szNameFile = entFile->d_name;
        p_Rec[iIdxFile].a_Hash = NULL;
        p_Rec[iIdxFile].a_Slc = NULL;
        p_Rec[iIdxFile].p_Conf = p_Conf;
        p_Rec[iIdxFile].plg_Slc = plg_Slc;
        pthread_create(&(p_Rec[iIdxFile].tIdThrd), NULL, _CrlHandleSlice, 
                       (void*)&(p_Rec[iIdxFile]));
        iIdxFile++;
    }

    /* Reduce the results and put them into MELT_POT structure. */
    for (iIdxFile = 0 ; iIdxFile < iCntFile ; iIdxFile++) {
        pthread_join(p_Rec[iIdxFile].tIdThrd, NULL);
    }
    sem_destroy(&ins_Sem);

FREERECORD:
    if (p_Rec != NULL) {
        for (iIdxFile = 0 ; iIdxFile < iCntFile ; iIdxFile++) {
            if (p_Rec[iIdxFile].a_Slc != NULL) {
                g_ptr_array_free (p_Rec[iIdxFile].a_Slc, TRUE);
            }
        }
        free(p_Rec);
    }

CLOSEDIR:
    if (dirRoot != NULL) {
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
    int8_t cRtnCode = CLS_SUCCESS;
    THREAD_SLICE *p_Param = (THREAD_SLICE*)vp_ThrdParam;

    /* Get the file slices. */
    char szPathFile[PATH_BUF_SIZE];
    snprintf(szPathFile, PATH_BUF_SIZE, "%s/%s", p_Param->p_Conf->szPathRootIn,
                                                 p_Param->szNameFile);
    int8_t cStat = p_Param->plg_Slc->GetFileSlice(szPathFile, p_Param->p_Conf->usSizeSlc, 
                                                  &(p_Param->a_Slc));
    if (cStat != SLC_SUCCESS) {
        EXITQ(CLS_FAIL_PLUGIN_INTERACT, EXIT);
    }
    sem_post(&ins_Sem);

EXIT:
    return;
}
