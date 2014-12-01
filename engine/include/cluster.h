#ifndef _CLUSTER_H_
#define _CLUSTER_H_


#include <stdint.h>


/* This ds records the paramters to control the clustering process. */
typedef struct _CONFIG_T {
    uint8_t ucCntThrd;      /* The number of threads used for parallel processing. */
    uint8_t ucScoreSim;     /* The threshold to group similar binary slices. */
    uint8_t ucCntBlk;       /* The number of hex byte blocks in pattern. */
    uint8_t ucSizeBlk;      /* The size of the hex byte block. */    
    uint8_t ucSizeSlc;      /* The size of the binary slice extracted from file
                               which is the basic unit of clustering process. */
    char *szPathRootIn;     /* The root pathname of the input sample set. */
    char *szPathRootOut;    /* The root pathname of the output patterns. */
    char *szPluginSlc;      /* The name of the plugin which is to be used for 
                               file slicing. */
    char *szPluginSim;      /* The name of the plugin which is to be used for 
                               similarity comparison. */
} CONFIG;


/**
 * This function initializes the clustering engine.
 *
 * @return status code
 */
int8_t
ClsInit();


/**
 * This function releases the resources allocated by clustering engine.
 *
 * @return status code
 */
int8_t
ClsDeinit();


/**
 * This function sets the task configuration specified by user.
 *
 * @param p_Config      The pointer to the user specified configuration
 *
 * @return status code
 */
int8_t
ClsSetConfig(CONFIG *p_Config);


/**
 * This function loads two major plugins: 
 *      1. The plugin for file slicing. 
 *      2. The plugin for similarity comparison
 *
 * @return status code
 */
int8_t
ClsLoadPlugin();


/**
 * This function launches the major clustering workflow.
 *
 * @return status code
 */
int8_t
ClsProcessSampleSet();


#endif
