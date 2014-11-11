#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <assert.h>
#include <fuzzy.h>
#include "ds.h"
#include "spew.h"
#include "group.h"
#include "utarray.h"

/* DOS(MZ) header related information. */
#define MZ_HEADER_SIZE                      (0x40) /* The size of DOS(MZ) header. */
#define MZ_HEADER_OFF_PE_HEADER_OFFSET      (0x3c) /* The starting offset of PE header. */

/* PE header related information. */
#define PE_HEADER_SIZE                      (0x18) /* The size of PE header. */
#define PE_HEADER_OFF_NUMBER_OF_SECTION     (0x6)  /* The number of sections. */
#define PE_HEADER_OFF_SIZE_OF_OPT_HEADER    (0x14) /* The size of PE optional header.*/

/* Section header related information. */
#define SECTION_HEADER_PER_ENTRY_SIZE       (0x28) /* The size of each section entry. */
#define SECTION_HEADER_OFF_RAW_SIZE         (0x10) /* The section raw size. */
#define SECTION_HEADER_OFF_RAW_OFFSET       (0x14) /* The section raw offset. */
#define SECTION_HEADER_OFF_CHARS            (0x24) /* The sectoin characteristics. */
#define SECTION_HEADER_NAME_SIZE            (0x8)  /* The maximum length of section name. */

/* The size definition of some data type. */
#define DATATYPE_SIZE_DWORD                 (4)
#define DATATYPE_SIZE_WORD                  (2)
#define SHIFT_RANGE_8BIT                    (8)

/*======================================================================*
 *                    Declaration for Private Object                    *
 *======================================================================*/
UT_array *_arraySample;
UT_array *_arrayBinary;


/*======================================================================*
 *                  Declaration for Internal Functions                  *
 *======================================================================*/

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
 *                    <0: Possible causes:
 *                          1. insufficient memory.
 *                          2. Invalid binary of certain samples.
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
 *                          1. insufficient memory.
 *                          2. Invalid binary of certain samples.
 */
int _grp_generate_section_hash(FILE *fpSample, SAMPLE *hSample);

/*======================================================================*
 *                Implementation for External Functions                 *
 *======================================================================*/
 
 /**
  * !EXTERNAL
  * grp_init_task(): The constructor of GROUP structure.
  */
int grp_init_task(GROUP *self, CONFIG *cfgTask) {

    self->cfgTask = cfgTask;
    self->generate_hash = grp_generate_hash;
    self->group_hash = grp_group_hash;
    return 0;
}

/**
 * !EXTERNAL
 * grp_deinit_task(): The destructor of GROUP structure.
 */
int grp_deinit_task(GROUP *self) {

    /* Free the array of SAMPLE structure. */
    utarray_free(_arraySample);
    return 0;
}

/**
 * !EXTERNAL
 * grp_generate_hash(): Generate section hashes for all the given samples.
 */
int grp_generate_hash(GROUP *self) {
    int      rc, i;
    SAMPLE   instSample;
    char     *pathRoot;
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
        Spew1("Error: %s", strerror(errno));
        rc = -1;
        goto EXIT;
    }

    /* Initialize the array to record per sample information. */
    UT_icd icdSample = {sizeof(SAMPLE), NULL, utarray_sample_copy, utarray_sample_deinit};
    utarray_new(_arraySample, &icdSample);

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
            Spew1("Error: %s", strerror(errno));
            rc = -1;
            goto CLOSE_DIR;
        }
        instSample.nameSample = entFile->d_name;
        
        /* Extract the section information from file header.*/
        rc = _grp_extract_section_info(fpSample, &instSample);
        if (rc != 0) {
            goto CLOSE_FILE;
        }

        /* Generate the hash of each section. */
        rc = _grp_generate_section_hash(fpSample, &instSample);
        if (rc != 0) {
            goto CLOSE_FILE;
        }

        /* Insert the SAMPLE structure into array. */
        utarray_push_back(_arraySample, &instSample);
    CLOSE_FILE:
        fclose(fpSample);
    }
  
CLOSE_DIR:
    closedir(dirRoot);
EXIT:
    return rc;
}

/**
 * !EXTERNAL
 * grp_group_hash(): Group the hashes using the given similarity threshold.
 */
int grp_group_hash(GROUP *self) {
    int rc;

    rc = 0;
    
      /*
    hSample = NULL;
    while ((hSample = (SAMPLE*)utarray_next(arraySample, hSample)) != NULL) {
        printf("%s\n", hSample->nameSample);
    }
    */
    
    return rc;
}


/*======================================================================*
 *                Implementation for Internal Functions                 *
 *======================================================================*/

/**
 * !INTERNAL
 * utarray_sample_copy(): Guide utarray for SAMPLE structure copy.
 */
void utarray_sample_copy(void *pTarget, const void *pSource) {
    uint16_t i, countSection;
    SAMPLE   *hTarget, *hSource;
    SECTION  *arraySecTge, *arraySecSrc;

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
        
        for (i = 0 ; i < countSection ; i++) {
            arraySecTge[i].rawOffset = arraySecSrc[i].rawOffset;
            arraySecTge[i].rawSize = arraySecSrc[i].rawSize;
            arraySecTge[i].hash = NULL;
            
            /* Copy the hash string. */
            if (arraySecSrc[i].hash != NULL) {
                arraySecTge[i].hash = strdup(arraySecSrc[i].hash);
                assert(arraySecTge[i].hash != NULL);
                free(arraySecSrc[i].hash);
            }
        }
        free(hSource->arraySection);
    }

    return;
}

/**
 * !INTERNAL
 * utarray_sample_deinit(): Guide utarray for SAMPLE structure release.
 */
void utarray_sample_deinit(void *pCurrent) {
    uint16_t i;
    SAMPLE   *hCurrent;
    
    /* Free the sample name. */
    hCurrent = (SAMPLE*)pCurrent;
    if (hCurrent->nameSample != NULL) {
        free(hCurrent->nameSample);
    }

    /* Free the array of SECTION structures. */
    if (hCurrent->arraySection != NULL) {
        for (i = 0 ; i < hCurrent->countSection ; i++) {
            if (hCurrent->arraySection[i].hash != NULL) {
                free(hCurrent->arraySection[i].hash);
            }
        }
        free(hCurrent->arraySection);
    }
    
    return;
}

/**
 * !INTERNAL
 * _grp_extract_section_info(): Extract the physical offset and size of each PE section.
 */
int _grp_extract_section_info(FILE *fpSample, SAMPLE *hSample) {
    int      rc;
    size_t   nExptRead, nRealRead;
    uint32_t dwordReg, offsetPEHeader;
    uint16_t i, j, wordReg, countSection;
    SECTION  *arraySection;
    char     buf[BUF_SIZE_LARGE];

    rc = 0;
    /* Check the MZ header. */
    nExptRead = MZ_HEADER_SIZE;
    nRealRead = fread(buf, sizeof(char), nExptRead, fpSample);
    if ((nExptRead != nRealRead) || (buf[0] != 'M') || (buf[1] != 'Z')) {
        Spew0("Error: Invalid PE file (Invalid MZ header).");
        rc = -1;
        goto EXIT;
    }

    /* Resolve the starting offset of PE header and move to it. */
    dwordReg = 0;
    for (i = 1 ; i <= DATATYPE_SIZE_DWORD ; i++) {
        dwordReg <<= SHIFT_RANGE_8BIT;
        dwordReg += buf[MZ_HEADER_OFF_PE_HEADER_OFFSET + DATATYPE_SIZE_DWORD - i] & 0xff;
    }
    offsetPEHeader = dwordReg;
    rc = fseek(fpSample, offsetPEHeader, SEEK_SET);
    if (rc != 0) {
        Spew0("Error: Invalid PE file (Unreachable PE header).");
        rc = -1;
        goto EXIT;
    }
      
    /* Check the PE header. */
    nExptRead = PE_HEADER_SIZE;
    nRealRead = fread(buf, sizeof(char), nExptRead, fpSample);
    if ((nExptRead != nRealRead) || (buf[0] != 'P' || buf[1] != 'E')) {
        Spew0("Invalid PE file (Invalid PE header).");
        rc = -1;
        goto EXIT;
    }
    
    /* Resolve the number of sections. */
    wordReg = 0;
    for (i = 1 ; i <= DATATYPE_SIZE_WORD ; i++) {
        wordReg <<= SHIFT_RANGE_8BIT;
        wordReg += buf[PE_HEADER_OFF_NUMBER_OF_SECTION + DATATYPE_SIZE_WORD - i] & 0xff;
    }
    countSection = wordReg;

    /* Resolve the size of optional header. */
    wordReg = 0;
    for (i = 1 ; i <= DATATYPE_SIZE_WORD ; i++) {
        wordReg <<= SHIFT_RANGE_8BIT;
        wordReg += buf[PE_HEADER_OFF_SIZE_OF_OPT_HEADER + DATATYPE_SIZE_WORD - i] & 0xff;
    }
        
    /* Move to the starting offset of section header. */
    rc = fseek(fpSample, (offsetPEHeader + PE_HEADER_SIZE + wordReg), SEEK_SET);
    if (rc != 0) {
        Spew0("Error: Invalid PE file (Unreachable section header).");
        rc = -1;
        goto EXIT;
    }
    
    hSample->countSection = countSection;
    hSample->arraySection = (SECTION*)malloc(sizeof(SECTION) * countSection);
    if (hSample->arraySection == NULL) {
        Spew0("Error: Cannot allocate an array of SECTION structures.");
        rc = -1;
        goto EXIT;
    }
    arraySection = hSample->arraySection;
    
    /* Traverse each section header to retrieve the raw section offset and size. */
    for (i = 0 ; i < countSection ; i++) {
        nExptRead = SECTION_HEADER_PER_ENTRY_SIZE;
        nRealRead = fread(buf, sizeof(char), nExptRead, fpSample);
        if (nExptRead != nRealRead) {
            Spew0("Invalid PE file (Invalid section header).");
            rc = -1;
            goto EXIT;
        }

        /* Record the raw section size. */
        dwordReg = 0;
        for (j = 1 ; j <= DATATYPE_SIZE_DWORD ; j++) {
            dwordReg <<= SHIFT_RANGE_8BIT;
            dwordReg += buf[SECTION_HEADER_OFF_RAW_SIZE + DATATYPE_SIZE_DWORD - j] & 0xff;
        }
        arraySection[i].rawSize = dwordReg;

        /* Record the raw section offset. */
        dwordReg = 0;
        for (j = 1 ; j <= DATATYPE_SIZE_DWORD ; j++) {
            dwordReg <<= SHIFT_RANGE_8BIT;
            dwordReg += buf[SECTION_HEADER_OFF_RAW_OFFSET + DATATYPE_SIZE_DWORD - j] & 0xff;
        }
        arraySection[i].rawOffset = dwordReg;
        arraySection[i].hash = NULL;
    }
    
EXIT:
    return rc;
}

/**
 * !INTERNAL
 * _grp_generate_section_hash(): Generate the hash of each PE section.
 */
int _grp_generate_section_hash(FILE *fpSample, SAMPLE *hSample) {
    int      rc;
    uint16_t i;
    uint32_t rawOffset, rawSize;
    size_t   nExptRead, nRealRead;
    char     *content;
    SECTION  *arraySection;

    rc = 0;
    /* Traverse all the sections with non-zero size. */
    arraySection = hSample->arraySection;
    for (i = 0 ; i < hSample->countSection ; i++) {
        rawOffset = arraySection[i].rawOffset;
        rawSize = arraySection[i].rawSize;
        if (rawSize == 0) {
            continue;
        }
        rc = fseek(fpSample, rawOffset, SEEK_SET);
        if (rc != 0) {
            Spew0("Error: Invalid PE file (Unreachable section).");
            rc = -1;
            goto EXIT;
        }
        content = (char*)malloc(sizeof(char) * rawSize);
        if (content == NULL) {
            Spew0("Error: Cannot allocate buffer for section content.");
            rc = -1;
            goto EXIT;
        }
        nExptRead = rawSize;
        nRealRead = fread(content, sizeof(char), nExptRead, fpSample);
        if (nExptRead != nRealRead) {
            Spew0("Error: Cannot read the full section content.");
            rc = -1;
            goto FREE;
        }
        arraySection[i].hash = (char*)malloc(sizeof(char) * FUZZY_MAX_RESULT);
        if (arraySection[i].hash == NULL) {
            Spew0("Error: Cannot allocate buffer for section hash.");
            rc = -1;
            goto FREE;
        }
        
        /* Apply the ssdeep library to generate section hash. */
        fuzzy_hash_buf(content, rawSize, arraySection[i].hash);
    FREE:
        free(content);    
    }

EXIT:
    return rc;
}
