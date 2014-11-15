#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ds.h"
#include "spew.h"
#include "pattern.h"


/*======================================================================*
 *                  Declaration for Internal Functions                  *
 *======================================================================*/


/*======================================================================*
 *                Implementation for External Functions                 *
 *======================================================================*/
/**
 * !EXTERNAL
 * PtnInitTask(): The constructor of PATTERN structure.
 */
int PtnInitTask(PATTERN *self, CONFIG *pCfg) {
    
    self->pCfg = pCfg;
    self->locateBinarySequence = PtnLocateBinarySequence;
    self->generatePattern = PtnGeneratePattern;    

    return 0;
}

/**
 * !EXTERNAL
 * PtnDeinitTask(): The destructor of PATTERN structure.
 */
int PtnDeinitTask(PATTERN *self) {

    return 0;
}

/**
 * !EXTERNAL
 * PtnLocateBinarySequence(): Select a set of candidates which represent the 
 * similar binary sequence of the clustered PE sections.
 */
int PtnLocateBinarySequence(PATTERN *self, GROUP_RESULT *pGrpRes) {
    int iRtnCode;
    uint8_t ucParallelity, ucBlkCount, ucBlkSize;
    uint32_t uiBinCount, uiGrpCount, uiSlotCount;
    UT_array *pABin;
    FAMILY *pMapFam;
    
    iRtnCode = 0;
    uiGrpCount = HASH_CNT(hh, pGrpRes->pMapFam);
    printf("%d\n", uiGrpCount);

    return iRtnCode;
}

/**
 * !EXTERNAL
 * PtnGeneratePattern(): Output the set of candidates each of which is outputted 
 * as a Yara-formatted pattern.
 */
int PtnGeneratePattern(PATTERN *self) {

    return 0;
}


/*======================================================================*
 *                Implementation for Internal Functions                 *
 *======================================================================*/
