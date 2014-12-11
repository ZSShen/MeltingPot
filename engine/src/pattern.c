#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
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
 * This function iteratively merges the extraction result.
 *
 * @param p_Param       The pointer to the result updated by Craft thread.
 */
int8_t
_PtnReduceCraft(THREAD_CRAFT *p_Param);


/**
 * This function initializes the THREAD_CRAFT structure.
 * 
 * @param p_Param       The pointer to the to be initialized structure.
 * @param p_Grp         The pointer to the GROUP structure.
 * 
 * @return status code
 */
int8_t
_PtnSetParamThrdCrt(THREAD_CRAFT *p_Param, GROUP *p_Grp);


/**
 * This function initializes the array of THREAD_CRAFT structures.
 * 
 * @param p_aParam      The pointer to the array of THREAD_CRAFT structures.
 * @param uiSize        The array size.
 * 
 * @return status code
 */
int8_t
_PtnInitArrayThrdCrt(THREAD_CRAFT **p_aParam, uint32_t uiSize);


/**
 * This function deinitializes the array of THREAD_CRAFT structures.
 * 
 * @param a_Param       The array of THREAD_CRAFT structures.
 * @param uiSize        The array size.
 * 
 * @return (currently unused)
 */
int8_t
_PtnDeinitArrayThrdCrt(THREAD_CRAFT *a_Param, uint32_t uiSize);


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
    int8_t cRtnCode;

    return cRtnCode;
}


int8_t
PtnOutputYara()
{
    int8_t cRtnCode;
    
    return cRtnCode;
}


/*======================================================================*
 *                Implementation for Internal Functions                 *
 *======================================================================*/
int8_t
_PtnSetParamThrdCrt(THREAD_CRAFT *p_Param, GROUP *p_Grp)
{
    int8_t cRtnCode = CLS_SUCCESS;    

    p_Param->p_Grp = p_Grp;
    p_Param->a_BlkCand = g_ptr_array_new();
    if (!p_Param->a_BlkCand)
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

EXIT:    
    return cRtnCode;
}


int8_t
_PtnInitArrayThrdCrt(THREAD_CRAFT **p_aParam, uint32_t uiSize)
{
    int8_t cRtnCode = CLS_SUCCESS;

    *p_aParam = (THREAD_CRAFT*)malloc(sizeof(THREAD_CRAFT) * uiSize);
    if (!(*p_aParam))
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    THREAD_CRAFT *a_Param = *p_aParam;
    uint32_t uiIdx;
    for (uiIdx = 0 ; uiIdx < uiSize ; uiIdx++) {
        a_Param[uiIdx].p_Grp = NULL;
        a_Param[uiIdx].a_BlkCand = NULL;
    }

EXIT:
    return cRtnCode;
}


int8_t
_PtnDeinitArrayThrdCrt(THREAD_CRAFT *a_Param, uint32_t uiSize)
{
    if (!a_Param)
        return CLS_SUCCESS;

    uint32_t uiIdx;
    for (uiIdx = 0 ; uiIdx < uiSize ; uiIdx++) {
        if (a_Param[uiIdx].a_BlkCand)
            g_ptr_array_free(a_Param[uiIdx].a_BlkCand, true);
    }

    free(a_Param);
    return CLS_SUCCESS;
}
