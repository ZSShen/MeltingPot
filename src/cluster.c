#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cluster.h"
#include "ds.h"
#include "spew.h"
#include "group.h"
#include "pattern.h"


/*-----------------------------------------------------------*
 *            Declaration for Private Objects                *
 *-----------------------------------------------------------*/
static GROUP    *_hGroup;
static PATTERN  *_hPattern;


/*-----------------------------------------------------------*
 *           Declaration for Internal Functions              *
 *-----------------------------------------------------------*/



/*-----------------------------------------------------------*
 *          Implementation for External Functions            *
 *-----------------------------------------------------------*/
int cls_init_task(CLUSTER *self) {

    self->init_ctx = cls_init_ctx;
    self->deinit_ctx = cls_deinit_ctx;
    self->generate_group = cls_generate_group;
    self->generate_pattern = cls_generate_pattern;
    return 0;
}


int cls_deinit_task(CLUSTER *self) {

    return 0;
}


int cls_init_ctx(CLUSTER *self, CONFIG *pCfg) {
    int iRtnCode;

    iRtnCode = 0;
    
    /* Initialize the handle of binary grouping. */
    INIT_GROUP(_hGroup, pCfg);
    if (_hGroup == NULL) {
        iRtnCode = -1;
        goto EXIT;
    }

EXIT:
    return iRtnCode;
}


int cls_deinit_ctx(CLUSTER *self) {
    int iRtnCode;

    iRtnCode = 0;

    /* Deinitialize the handle of binary grouping.  */
    DEINIT_GROUP(_hGroup);

    return iRtnCode;
}


int cls_generate_group(CLUSTER *self) {
    int iRtnCode;

    iRtnCode = 0;
    
    /* Generate the section hashes for the given sample set. */
    iRtnCode = _hGroup->generate_hash(_hGroup);
    if (iRtnCode != 0) {
        goto EXIT;
    }

    /* Group the similar hashes using the given threshold. */
    iRtnCode = _hGroup->group_hash(_hGroup);
    if (iRtnCode != 0) {
        goto EXIT;
    }

EXIT:
    return iRtnCode;
}


int cls_generate_pattern(CLUSTER *self) {
    int iRtnCode;

    iRtnCode = 0;

    return iRtnCode;
}


/*-----------------------------------------------------------*
 *          Implementation for Internal Functions            *
 *-----------------------------------------------------------*/
