#ifndef _GROUP_H_
#define _GROUP_H_

#include "cluster.h"

typedef struct _GROUP {
    CONFIG *pCfg;

    int (*generate_hash) (struct _GROUP*);
    int (*group_hash)    (struct _GROUP*);
} GROUP;

/* The wrapper for GROUP constructor. */
#define INIT_GROUP(p, q)    p = (GROUP*)malloc(sizeof(GROUP));   \
                            if (p != NULL) {                     \
                                iRtnCode = GrpInitTask(p, q);        \
                                if (iRtnCode == -1) {                  \
                                    free(p);                     \
                                }                                \
                            }

/* The wrapper for GROUP destructor. */
#define DEINIT_GROUP(p)     if (p != NULL) {                     \
                                GrpDeinitTask(p);              \
                                free(p);                         \
                            }

/**
 * The constructor of GROUP structure.
 * 
 * @param self      The pointer to the GROUP structure.
 * @param pCfg   The pointer to the task configuration.
 *
 * @return          0: Task is finished successfully.
 *                 <0: Currently undefined. 
 */
int GrpInitTask(GROUP *self, CONFIG *pCfg);

/**
 * The destructor of GROUP structure.
 * 
 * @param self      The pointer to the GROUP structure.
 * 
 * @return          0: Task is finished successfully.
 *                 <0: Currently undefined.
 */
int GrpDeinitTask(GROUP *self);

/**
 * This function generates section hashes for all the given samples.
 *  
 * @param self      The pointer to the GROUP structure.
 * 
 * @return          0: Task is finished successfully.
 *                 <0: Possible causes:
 *                      1. Insufficient memory.
 *                      2. Invalid binary format of certain samples.
 *                      3. IO error.
 */ 
int GrpGenerateHash(GROUP *self);

/**
 * This function groups the hashes using the given similarity threshold.
 *  
 * @param self      The pointer to the GROUP structure.
 * 
 * @return          0: Task is finished successfully.
 *                 <0: Possible cause:
 *                      1. Insufficient memory.
 */ 
int GrpCorrelateHash(GROUP *self);

#endif
