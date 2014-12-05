#ifndef _CORRELATE_H_
#define _CORRELATE_H_


#include <stdint.h>
#include "data.h"
#include "cluster.h"
#include "slice.h"
#include "similarity.h"


#define PATH_BUF_SIZE   1024


/* The thread parameter to record the result of file slicing. */
typedef struct THREAD_SLICE_T {
    pthread_t tId;
    uint16_t usSizeSlc;
    char *szPath;
    GPtrArray *a_Hash;
    GPtrArray *a_Slc;
    PLUGIN_SLICE *plg_Slc;
    PLUGIN_SIMILARITY *plg_Sim;
} THREAD_SLICE;


/**
 * This function sets the context which:
 *     1. Provides the user specified configuration and plugins.
 *     2. Should be updated with he clustering progress.
 *
 * @param p_Ctx     The pointer to the CONTEXT structure.
 * 
 * @return (currently unused)
 */
int8_t
CrlSetContext(CONTEXT *p_Ctx);


/**
 * This function processes the given set of files, each of which:
 *     1. Will be divided into slices.
 *     2. Each slice will be hashed into a string for similarity comparison.
 * 
 * @return status code
 */
int8_t
CrlPrepareSlice();


/**
 * This function correlates the similar slices into groups and extracts the common 
 * features shared by each group.
 *
 * @return status code
 */
int8_t
CrlCorrelateSlice();


#endif
