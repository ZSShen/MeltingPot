#ifndef _CLUSTER_H_
#define _CLUSTER_H_


#include <stdint.h>


/*======================================================================*
 *                  Declaration for Shared Interfaces                   *
 *======================================================================*/
/* The ds which exports configuration parameters for external modules to specify
   the task environment. */
typedef struct _CONFIG {
    uint8_t ucParallelity;
    uint8_t ucBlkCount;
    uint8_t ucBlkSize;
    uint8_t ucSimilarity;
    char *szPathInput;
    char *szPathOutput;
} CONFIG;


/* This module controls the work flow of binary clustering and pattern generation. */
typedef struct _CLUSTER {
    int (*initCtx) (struct _CLUSTER*, CONFIG*);
    int (*deinitCtx) (struct _CLUSTER*);
    int (*generateGroup) (struct _CLUSTER*);
    int (*generatePattern) (struct _CLUSTER*);
    void (*showUsage) (struct _CLUSTER*); 
} CLUSTER;


/* The wrapper for CLUSTER constructor. */
#define INIT_CLUSTER(p)     p = (CLUSTER*)malloc(sizeof(CLUSTER));     \
                            if (p != NULL) {                           \
                                iRtnCode = ClsInitTask(p);             \
                                if (iRtnCode == -1) {                  \
                                    free(p);                           \
                                }                                      \
                            }

/* The wrapper for CLUSTER destructor. */
#define DEINIT_CLUSTER(p)   if (p != NULL) {                           \
                                ClsDeinitTask(p);                      \
                                free(p);                               \
                            }


/*======================================================================*
 *                  Declaration for External Functions                  *
 *======================================================================*/
/**
 * The constructor of CLUSTER structure.
 * 
 * @param self      The pointer to the CLUSTER structure.
 *
 * @return          0: Task is finished successfully.
 *                 <0: Currently undefined.
 */
int ClsInitTask(CLUSTER *self);

/**
 * The destructor of CLUSTER structure.
 * 
 * @param self      The pointer to the CLUSTER structure.
 * 
 * @return          0: Task is finished successfully.
 *                 <0: Currently undefined.
 */
int ClsDeinitTask(CLUSTER *self);

/**
 * This function creates the GROUP and PATTERN modules and passes the task 
 * configuration to them.
 * 
 * @param self      The pointer to the CLUSTER structure.
 * @param pCfg      The pointer to the task configuration.
 * 
 * @return          0: Task is finished successfully.
 *                 <0: Possible causes:
 *                      1. Invalid task configuration.
 *                      2. Insufficient memory.
 */
int ClsInitCtx(CLUSTER *self, CONFIG *pCfg);

/**
 * This function cleans up the GROUP and PATTERN modules.
 * 
 * @param self      The pointer to the CLUSTER structure.
 * 
 * @return          0: Task is finished successfully.
 *                 <0: Currently undefined.
 */ 
int ClsDeinitCtx(CLUSTER *self);

/**
 * This function triggers the GROUP module for binary clustering.
 * 
 * @param self      The pointer to the CLUSTER structure.
 * 
 * @return          0: Task is finished successfully.
 *                 <0: Errors triggered by GROUP module.
 */
int ClsGenerateGroup(CLUSTER *self);

/**
 * This function triggers the PATTERN module for pattern generation.
 * 
 * @param self      The pointer to the CLUSTER structure.
 *
 * @return          0: Task is finished successfully.
 *                 <0: Errors triggerd by PATTERN module.
 */
int ClsGeneratePattern(CLUSTER *self);


/**
 * This function shows the usage guide of the engine.
 * 
 * @param self      The pointer to the CLUSTER structure.
 * 
 */
void ClsPrintUsage(CLUSTER *self);
 

#endif
