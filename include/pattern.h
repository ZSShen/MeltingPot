#ifndef _PATTERN_H_
#define _PATTERN_H_


#include "cluster.h"


/*======================================================================*
 *                  Declaration for Shared Interfaces                   *
 *======================================================================*/
/* This module handles the pattern generation for clustered PE sections. */
typedef struct _PATTERN {
    CONFIG *pCfg;

    int (*locateBinarySequence) (struct _PATTERN*, GROUP_RESULT*);
    int (*generatePattern)      (struct _PATTERN*);
} PATTERN;


/* The wrapper for PATTERN constructor. */
#define INIT_PATTERN(p, q)  p = (PATTERN*)malloc(sizeof(PATTERN));     \
                            if (p != NULL) {                           \
                                iRtnCode = PtnInitTask(p, q);          \
                                if (iRtnCode == -1) {                  \
                                    free(p);                           \
                                }                                      \
                            }

/* The wrapper for PATTERN destructor. */
#define DEINIT_PATTERN(p)   if (p != NULL) {                           \
                                PtnDeinitTask(p);                      \
                                free(p);                               \
                            }
                            

/*======================================================================*
 *                  Declaration for External Functions                  *
 *======================================================================*/
/**
 * The constructor of PATTERN structure.
 * 
 * @param self      The pointer to the PATTERN structure.
 * @param pCfg      The pointer to the task configuration.
 *
 * @return          0: Task is finished successfully.
 *                 <0: Possible Cause:
 *                     1. Insufficient memory.
 */
int PtnInitTask(PATTERN *self, CONFIG *pCfg);

/**
 * The destructor of PATTERN structure.
 * 
 * @param self      The pointer to the PATTERN structure.
 * 
 * @return          0: Task is finished successfully.
 *                 <0: Currently undefined.
 */
int PtnDeinitTask(PATTERN *self);

/**
 * This function selects a set of candidates which represent the similar
 * binary sequence of the clustered PE sections.
 *  
 * @param self      The pointer to the PATTERN structure.
 * @param pGrpRes   The pointer to the section clusters.
 * 
 * @return          0: Task is finished successfully.
 *                 <0: Possible causes:
 *                      1. Insufficient memory.
 *                      2. IO error.
 */
int ptnLocateBinarySequence(PATTERN *self, GROUP_RESULT *pGrpRes);

/**
 * This function outputs the set of candidates each of which is outputted
 * as a Yara-formatted pattern.
 *  
 * @param self      The pointer to the PATTERN structure.
 * 
 * @return          0: Task is finished successfully.
 *                 <0: Possible causes:
 *                      1. Insufficient memory.
 *                      2. IO error.
 */
int ptnGeneratePattern(PATTERN *self);


#endif
