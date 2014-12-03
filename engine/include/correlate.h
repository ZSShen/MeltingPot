#ifndef _CORRELATE_H_
#define _CORRELATE_H_


#include <stdint.h>
#include "data.h"
#include "cluster.h"
#include "slice.h"
#include "similarity.h"


/* The thread parameter to record the result of file slicing. */
typedef struct THREAD_SLICE_T {
    pthread_t tIdThrd;
    char *szPathFile;
    GPtrArray *a_Hash;
    GPtrArray *a_Slc;
    PLUGIN_SLICE *plg_Slc;
} THREAD_SLICE;


/**
 * This function slices the given set of files via designated plugin.
 *
 * @param p_Pot     The pointer to the structure which records the clustering progress.
 * @param p_Conf    The pointer to the clustering configuration.
 * @param plg_Slc   The pointer to the file slicing plugin.
 *
 * @return status code
 */
int8_t
CrlPrepareSlice(MELT_POT *p_Pot, CONFIG *p_Conf, PLUGIN_SLICE *plg_Slc);


/**
 * This function correlates the similar slices into groups and extracts the common 
 * features shared by each group.
 *
 * @param p_Pot     The pointer to the structure which records the clustering progress.
 * @param p_Conf    The pointer to the clustering configuration.
 * @param plg_Sim   The pointer to the similarity computation plugin.
 *
 * @return status code
 */
int8_t
CrlCorrelateSlice(MELT_POT *p_Pot, CONFIG *p_Conf, PLUGIN_SIMILARITY *plg_Sim);


#endif
