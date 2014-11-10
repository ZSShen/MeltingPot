#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <assert.h>
#include "ds.h"
#include "spew.h"
#include "group.h"
#include "utarray.h"


/*-----------------------------------------------------------*
 *           Declaration for Internal Functions              *
 *-----------------------------------------------------------*/

/**
 * The utility to guide utarray for element copy.
 *
 * @param pTge     The pointer to the target.
 * @param pSrc     The pointer to the source object.
 */
void utarray_copy(void *pTge, const void *pSrc);


/**
 * The utility to guide utarray for resource release.
 *
 * @param pCur     The pointer to the to be released object.
 */
void utarray_deinit(void *pCur);





/*-----------------------------------------------------------*
 *          Implementation for External Functions            *
 *-----------------------------------------------------------*/
int grp_init_task(GROUP *self, CONFIG *cfgTask) {

    self->cfgTask = cfgTask;
    self->generate_hash = grp_generate_hash;
    self->group_hash = grp_group_hash;
    return 0;
}


int grp_deinit_task(GROUP *self) {

    return 0;
}


int grp_generate_hash(GROUP *self) {
    int      rc;
    SAMPLE   instSample;
    UT_array *arrSample;
    DIR      *dirRoot;
    SAMPLE   *hSample;
    struct dirent *entFile;

    rc = 0;

    /* Open the root path of designated sample set. */
    dirRoot = opendir(self->cfgTask->cfgPathRoot);
    if (dirRoot == NULL) {
        rc = -1;
        Spew1("Error: %s", strerror(errno));
        goto EXIT;
    }

    /* Initialize the array to record per sample information. */
    UT_icd icdUtArray = {sizeof(SAMPLE), NULL, utarray_copy, utarray_deinit};
    utarray_new(arrSample, &icdUtArray);

    /* Traverse each sample for section hash generation. */
    while ((entFile = readdir(dirRoot)) != NULL) {
        instSample.countSection = 0;
        instSample.arrSection = NULL;
        instSample.nameSample = entFile->d_name;
        utarray_push_back(arrSample, &instSample);
    }

    hSample = NULL;
    while ((hSample = (SAMPLE*)utarray_next(arrSample, hSample)) != NULL) {
        printf("%s\n", hSample->nameSample);
    }

FREE:
    utarray_free(arrSample);
    closedir(dirRoot);

EXIT:
    return rc;
}


int grp_group_hash(GROUP *self) {
    int rc;

    rc = 0;

    return rc;
}


/*-----------------------------------------------------------*
 *          Implementation for Internal Functions            *
 *-----------------------------------------------------------*/
/**
 * utarray_copy(): Guide utarray for element copy.
 */
void utarray_copy(void *pTge, const void *pSrc) {
    int    idxSection, countSection;
    SAMPLE *hTge, *hSrc;

    hTge = (SAMPLE*)pTge;
    hSrc = (SAMPLE*)pSrc;

    /* Copy the section count. */
    hTge->countSection = hSrc->countSection;
    hTge->nameSample = NULL;
    hTge->arrSection = NULL;

    /* Copy the sample name. */
    if (hSrc->nameSample != NULL) {
        hTge->nameSample = strdup(hSrc->nameSample);
        assert(hTge->nameSample != NULL);
    }

    /* Copy the array of SECTION structures. */
    if (hSrc->arrSection != NULL) {
        countSection = hTge->countSection;
        hTge->arrSection = (SECTION*)malloc(sizeof(SECTION) * countSection);
        assert(hTge->arrSection != NULL);
        
        for (idxSection = 0 ; idxSection < countSection ; idxSection++) {
            hTge->arrSection[idxSection].offsetRaw = hSrc->arrSection[idxSection].offsetRaw;
            hTge->arrSection[idxSection].sizeRaw = hSrc->arrSection[idxSection].sizeRaw;
            
            /* Copy the hash string. */
            if (hSrc->arrSection[idxSection].hash != NULL) {
                hTge->arrSection[idxSection].hash = strdup(hSrc->arrSection[idxSection].hash);
                assert(hTge->arrSection[idxSection].hash != NULL);
            }
        }
    }

    return;
}


/**
 * utarray_deinit(): Guide utarray for resource release.
 */
void utarray_deinit(void *pCur) {
    int    idxSection;
    SAMPLE *hCur;
    
    hCur = (SAMPLE*)pCur;
    
    /* Free the sample name. */
    if (hCur->nameSample != NULL) {
        free(hCur->nameSample);
    }

    /* Free the array of SECTION structures. */
    if (hCur->arrSection != NULL) {
        for (idxSection = 0 ; idxSection < hCur->countSection ; idxSection++) {
            if (hCur->arrSection[idxSection].hash != NULL) {
                free(hCur->arrSection[idxSection].hash);
            }
        }
        free(hCur->arrSection);
    }

    return;
}
