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
 * The utility to guide utarray for SAMLE structure copy.
 *
 * @param pTarget     The pointer to the target.
 * @param pSource     The pointer to the source object.
 */
void utarray_sample_copy(void *pTarget, const void *pSource);


/**
 * The utility to guide utarray for SAMPLE structure release.
 *
 * @param pCurrent     The pointer to the to be released object.
 */
void utarray_sample_deinit(void *pCurrent);


/**
 * This function extracts the physical offset and size of each PE section.
 *
 * @param fpSample     The file pointer to the analyzed sample.
 * @param hSample      The pointer to the SAMPLE structure storing
 *                     the section information.
 *
 * @return             0: Task is finished successfully.
 *                    <0: Invalid file format or insufficient memory.
 */
int _grp_extract_section_info(FILE *fpSample, SAMPLE *hSample);


/**
 * This function generate the hash of each PE section.
 *
 * @param fpSample     The file pointer to the analyzed sample.
 * @param hSample      The pointer to the SAMPLE structure storing
 *                     the section hash.
 *
 * @return             0: Task is finished successfully.
 *                    <0: Invalid file format or insufficient memory.
 */
int _grp_generate_section_hash(FILE *fpSample, SAMPLE *hSample);


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
    char     *pathRoot;
    UT_array *arraySample;
    DIR      *dirRoot;
    SAMPLE   *hSample;
    struct dirent *entFile;
    FILE     *fpSample;
    char     pathSample[BUF_SIZE_MEDIUM];

    rc = 0;

    /* Open the root path of designated sample set. */
    pathRoot = self->cfgTask->cfgPathRoot;
    dirRoot = opendir(pathRoot);
    if (dirRoot == NULL) {
        rc = -1;
        Spew1("Error: %s", strerror(errno));
        goto EXIT;
    }

    /* Initialize the array to record per sample information. */
    UT_icd icdSample = {sizeof(SAMPLE), NULL, utarray_sample_copy, utarray_sample_deinit};
    utarray_new(arraySample, &icdSample);

    /* Traverse each sample for section hash generation. */
    while ((entFile = readdir(dirRoot)) != NULL) {
        if ((strcmp(entFile->d_name, ".") == 0) || 
            (strcmp(entFile->d_name, "..") == 0)) {
            continue;
        }
        memset(pathSample, 0, sizeof(char) * BUF_SIZE_MEDIUM);
        snprintf(pathSample, BUF_SIZE_MEDIUM, "%s\%s", pathRoot, entFile->d_name);
        fpSample = fopen(pathSample, "rb");
        if (fpSample == NULL) {
            rc = -1;
            Spew1("Error: %s", strerror(errno));
            goto FREE;
        }
        
        /* Extract the section information from file header.*/
        rc = _grp_extract_section_info(fpSample, &instSample);
        if (rc != 0) {
            goto FREE;
        }

        /* Generate the hash of each section. */
        rc = _grp_generate_section_hash(fpSample, &instSample);

        instSample.countSection = 0;
        instSample.arraySection = NULL;
        instSample.nameSample = entFile->d_name;
        utarray_push_back(arraySample, &instSample);

        fclose(fpSample);
    }

    /*
    hSample = NULL;
    while ((hSample = (SAMPLE*)utarray_next(arraySample, hSample)) != NULL) {
        printf("%s\n", hSample->nameSample);
    }
    */

FREE:
    utarray_free(arraySample);
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
 * utarray_sample_copy(): Guide utarray for SAMPLE structure copy.
 */
void utarray_sample_copy(void *pTarget, const void *pSource) {
    int        idxSection, countSection;
    SAMPLE     *hTarget, *hSource;
    SECTION *arraySecTge, *arraySecSrc;

    /* Copy the section count. */
    hTarget = (SAMPLE*)pTarget;
    hSource = (SAMPLE*)pSource;
    hTarget->countSection = hSource->countSection;
    hTarget->nameSample = NULL;
    hTarget->arraySection = NULL;

    /* Copy the sample name. */
    if (hSource->nameSample != NULL) {
        hTarget->nameSample = strdup(hSource->nameSample);
        assert(hTarget->nameSample != NULL);
    }

    /* Copy the array of SECTION structures. */
    if (hSource->arraySection != NULL) {
        countSection = hTarget->countSection;
        hTarget->arraySection = (SECTION*)malloc(sizeof(SECTION) * countSection);
        assert(hTarget->arraySection != NULL);
        arraySecTge = hTarget->arraySection;
        arraySecSrc = hSource->arraySection;
        
        for (idxSection = 0 ; idxSection < countSection ; idxSection++) {
            arraySecTge[idxSection].offsetRaw = arraySecSrc[idxSection].offsetRaw;
            arraySecTge[idxSection].sizeRaw = arraySecSrc[idxSection].sizeRaw;
            
            /* Copy the hash string. */
            if (arraySecSrc[idxSection].hash != NULL) {
                arraySecTge[idxSection].hash = strdup(arraySecSrc[idxSection].hash);
                assert(arraySecTge[idxSection].hash != NULL);
            }
        }
    }

    return;
}


/**
 * utarray_sample_deinit(): Guide utarray for SAMPLE structure release.
 */
void utarray_sample_deinit(void *pCurrent) {
    int    idxSection;
    SAMPLE *hCurrent;
    
    /* Free the sample name. */
    hCurrent = (SAMPLE*)pCurrent;
    if (hCurrent->nameSample != NULL) {
        free(hCurrent->nameSample);
    }

    /* Free the array of SECTION structures. */
    if (hCurrent->arraySection != NULL) {
        for (idxSection = 0 ; idxSection < hCurrent->countSection ; idxSection++) {
            if (hCurrent->arraySection[idxSection].hash != NULL) {
                free(hCurrent->arraySection[idxSection].hash);
            }
        }
        free(hCurrent->arraySection);
    }

    return;
}


/**
 * _grp_extract_section_info(): Extract the physical offset and size of each PE section.
 */
int _grp_extract_section_info(FILE *fpSample, SAMPLE *hSample) {
    int rc;

    rc = 0;

    return rc;
}


/**
 * _grp_generate_section_hash(): Generate the hash of each PE section.
 */
int _grp_generate_section_hash(FILE *fpSample, SAMPLE *hSample) {
    int rc;

    rc = 0;

    return rc;
}
