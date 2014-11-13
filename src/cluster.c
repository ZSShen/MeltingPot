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
int ClsInitTask(CLUSTER *self) {

    self->initCtx = ClsInitCtx;
    self->deinitCtx = ClsDeinitCtx;
    self->generateGroup = ClsGenerateGroup;
    self->generatePattern = ClsGeneratePattern;
    return 0;
}


int ClsDeinitTask(CLUSTER *self) {

    return 0;
}


int ClsInitCtx(CLUSTER *self, CONFIG *pCfg) {
    int iRtnCode;

    iRtnCode = 0;
    
    /* Initialize the handle of binary grouping. */
    INIT_GROUP(_pGroup, pCfg);
    if (_pGroup == NULL) {
        iRtnCode = -1;
        goto EXIT;
    }

EXIT:
    return iRtnCode;
}


int ClsDeinitCtx(CLUSTER *self) {
    int iRtnCode;

    iRtnCode = 0;

    /* Deinitialize the handle of binary grouping.  */
    DEINIT_GROUP(_pGroup);

    return iRtnCode;
}


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


int ClsGeneratePattern(CLUSTER *self) {
    int iRtnCode;

    iRtnCode = 0;

    return iRtnCode;
}


/*======================================================================*
 *                Implementation for Internal Functions                 *
 *======================================================================*/
