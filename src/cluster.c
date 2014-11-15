#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cluster.h"
#include "ds.h"
#include "spew.h"
#include "group.h"
#include "pattern.h"


/*======================================================================*
 *                    Declaration for Private Object                    *
 *======================================================================*/
static GROUP *_pGroup;
static PATTERN *_pPattern;
static GROUP_RESULT *_pGrpRes;


/*======================================================================*
 *                  Declaration for Internal Functions                  *
 *======================================================================*/



/*======================================================================*
 *                Implementation for External Functions                 *
 *======================================================================*/
/**
 * !EXTERNAL
 * ClsInitTask(): The constructor of CLUSTER structure.
 */
int ClsInitTask(CLUSTER *self) {

    _pGroup = NULL;
    _pPattern = NULL;
    _pGrpRes = NULL;
    self->initCtx = ClsInitCtx;
    self->deinitCtx = ClsDeinitCtx;
    self->generateGroup = ClsGenerateGroup;
    self->generatePattern = ClsGeneratePattern;
    self->showUsage = ClsPrintUsage;
    
    return 0;
}

/**
 * !EXTERNAL
 * ClsDeinitTask(): The destructor of CLUSTER structure.
 */
int ClsDeinitTask(CLUSTER *self) {

    return 0;
}

/**
 * !EXTERNAL
 * ClsInitCtx(): Create the GROUP and PATTERN modules and passes the task 
 * configuration to them.
 */
int ClsInitCtx(CLUSTER *self, CONFIG *pCfg) {
    int iRtnCode;

    #define TERMINATE       iRtnCode = -1;                             \
                            goto EXIT;

    iRtnCode = 0;
    /* Inspect the task configuration. If it does not fit the standard, 
       terminate the entire workflow. */
    if (pCfg->ucParallelity < 1) {
        TERMINATE;
    }
    if (pCfg->ucSimilarity < 0) {
        TERMINATE;
    }
    if (pCfg->ucBlkCount < 1) {
        TERMINATE;
    }
    if (pCfg->ucBlkSize < 1) {
        TERMINATE;
    }
    if (pCfg->szPathInput == NULL) {
        TERMINATE;
    }
    if (pCfg->szPathOutput == NULL) {
        TERMINATE;
    }
    
    /* Initialize the GROUP module. */
    INIT_GROUP(_pGroup, pCfg);
    if (_pGroup == NULL) {
        TERMINATE;
    }
    /* Initialize the PATTERN module.*/
    INIT_PATTERN(_pPattern, pCfg);
    if (_pPattern == NULL) {
        TERMINATE;
    }

EXIT:
    return iRtnCode;
}

/**
 * !EXTERNAL
 * ClsDeinitCtx(): Clean up the GROUP and PATTERN modules.
 */
int ClsDeinitCtx(CLUSTER *self) {
    int iRtnCode;

    iRtnCode = 0;
    /* Deinitialize the GROUP module. */
    if (_pGroup != NULL) {
        DEINIT_GROUP(_pGroup);
    }
    /* Deinitialize the PATTERN module. */
    if (_pPattern != NULL) {
        DEINIT_PATTERN(_pPattern);
    }
    
    return iRtnCode;
}

/**
 * !EXTERNAL
 * ClsGenerateGroup(): Trigger the GROUP module for binary clustering.
 */
int ClsGenerateGroup(CLUSTER *self) {
    int iRtnCode;

    iRtnCode = 0;
    /* Generate the section hashes for the given sample set. */
    iRtnCode = _pGroup->generateHash(_pGroup);
    if (iRtnCode != 0) {
        goto EXIT;
    }

    /* Group the similar hashes using the given threshold. */
    iRtnCode = _pGroup->groupHash(_pGroup);
    if (iRtnCode != 0) {
        goto EXIT;
    }

    /* Cache the clustering result. */
    _pGrpRes = _pGroup->pGrpRes;
    
EXIT:
    return iRtnCode;
}

/**
 * !EXTERNAL
 * ClsGeneratePattern(): Trigger the PATTERN module for pattern generation.
 */ 
int ClsGeneratePattern(CLUSTER *self) {
    int iRtnCode;

    iRtnCode = 0;
    
    iRtnCode = _pPattern->locateBinarySequence(_pPattern, _pGrpRes);
    if (iRtnCode != 0) {
        goto EXIT;
    }

EXIT:
    return iRtnCode;
}

/**
 * !EXTERNAL
 * ClsPrintUsage(): Show the usage guide of the engine.
 */
void ClsPrintUsage(CLUSTER *self) {

    const char *strMsg = "Usage: entry --input path_in --output path_out --parallelity num_p --similarity num_t"
                         " --blkcount num_c --blksize num_s\n"
                         "       entry -i      path_in -o       path_out -p            num_p -t           num_t"
                         " -c         num_c -s        num_s\n\n"
                         "Example: entry --input /path/set --output /path/pattern --parallelity 2 --similarity 80"
                         " --blkcount 4 --blksize 16\n"
                         "         entry -i      /path/set -o       /path/pattern -p            2 -t           80"
                         " -c         4 -s        16\n\n";
    printf("%s", strMsg);
    return;
}

/*======================================================================*
 *                Implementation for Internal Functions                 *
 *======================================================================*/
