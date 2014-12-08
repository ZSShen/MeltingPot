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
_CrlMapCraft(void *vp_Param);


/**
 * This function iteratively merges the extraction result.
 *
 * @param p_Param       The pointer to the result updated by Craft thread.
 */
int8_t
_CrlReduceCraft(THREAD_CRAFT *p_Param);


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
 
