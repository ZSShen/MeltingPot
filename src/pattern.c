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
 * ptnLocateBinarySequence(): This function selects a set of candidates which 
 * represent the similar binary sequence of the clustered PE sections.
 */
int ptnLocateBinarySequence(PATTERN *self, GROUP_RESULT *pGrpRes) {

    return 0;
}

/**
 * !EXTERNAL
 * ptnGeneratePattern(): This function outputs the set of candidates each of 
 * which is outputted as a Yara-formatted pattern.
 */
int ptnGeneratePattern(PATTERN *self) {

    return 0;
}


/*======================================================================*
 *                Implementation for Internal Functions                 *
 *======================================================================*/
