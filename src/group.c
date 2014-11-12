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
UT_array *_arrayBinary;

/*======================================================================*
 *                  Declaration for Internal Functions                  *
 *======================================================================*/

/**
 * The utility to guide utarray for BINARY structure copy.
 *
 * @param pTarget     The pointer to the target.
 * @param pSource     The pointer to the source object.
 */
void utarray_binary_copy(void *pTarget, const void *pSource);

/**
 * The utility to guide utarray for BINARY structure release.
 *
 * @param pCurrent     The pointer to the to be released object.
 */
void utarray_binary_deinit(void *pCurrent);

/**
 * This function extracts the physical offset and size of each PE section.
 *
 * @param fpSample     The file pointer to the analyzed sample.
 * @param nameSample   The name of the given PE.
 * @param pIdBinary    The pointer to the binary id.
 *
 * @return             0: Task is finished successfully.
 *                    <0: Possible causes:
 *                          1. insufficient memory.
 *                          2. Invalid binary of certain samples.
 */
int _grp_extract_section_info(FILE *fpSample, char *nameSample, uint32_t *pIdBinary);

/**
 * This function generate the hash of each PE section.
 *
 * @param fpSample     The file pointer to the analyzed sample.
 * @param idBgnBin     The beginning binary id of the given PE.
 * @param idEndBin     The ending binary id of the given PE.
 *
 * @return             0: Task is finished successfully.
 *                    <0: Invalid file format or insufficient memory.
 *                          1. insufficient memory.
 *                          2. Invalid binary of certain samples.
 */
int _grp_generate_section_hash(FILE *fpSample, uint32_t idBgnBin, uint32_t idEndBin);

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
    utarray_free(_arrayBinary);
    return 0;
}

/**
 * !EXTERNAL
 * grp_generate_hash(): Generate section hashes for all the given samples.
 */
int grp_generate_hash(GROUP *self) {
    int      rc;
    uint32_t idBgnBin, idEndBin;
    char     *pathRoot;
    DIR      *dirRoot;
    FILE     *fpSample;
    struct dirent *entFile;
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
    UT_icd icdBinary = {sizeof(BINARY), NULL, utarray_binary_copy, utarray_binary_deinit};
    utarray_new(_arrayBinary, &icdBinary);

    /* Traverse each sample for section hash generation. */
    idBgnBin = idEndBin = 0;
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
        
        /* Extract the section information from file header.*/
        rc = _grp_extract_section_info(fpSample, entFile->d_name, &idEndBin);
        if (rc != 0) {
            goto CLOSE_FILE;
        }

        /* Generate the hash of each section. */
        rc = _grp_generate_section_hash(fpSample, idBgnBin, idEndBin);
        if (rc != 0) {
            goto CLOSE_FILE;
        }
        idBgnBin = idEndBin;

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
        
    return rc;
}


/*======================================================================*
 *                Implementation for Internal Functions                 *
 *======================================================================*/

/**
 * !INTERNAL
 * utarray_binary_copy(): Guide utarray for BINARY structure copy.
 */
void utarray_binary_copy(void *pTarget, const void *pSource) {
    BINARY *binSource, *binTarget;
    
    binSource = (BINARY*)pSource;
    binTarget = (BINARY*)pTarget;
    
    binTarget->idBinary = binSource->idBinary;
    binTarget->idxSection = binSource->idxSection;
    binTarget->offsetSection = binSource->offsetSection;
    binTarget->sizeSection = binSource->sizeSection;
    binTarget->nameSample = binSource->nameSample;
    if (binSource->hash != NULL) {
        binTarget->hash = strdup(binSource->hash);
        assert(binTarget->hash != NULL);
        free(binSource->hash);
    } else {
        binTarget->hash = NULL;
    }

    return;
}

/**
 * !INTERNAL
 * utarray_binary_deinit(): Guide utarray for BINARY structure release.
 */
void utarray_binary_deinit(void *pCurrent) {
    BINARY *hBinary;
    
    hBinary = (BINARY*)pCurrent;
    if (hBinary->hash != NULL) {
        free(hBinary->hash);
    }

    return;
}

/**
 * !INTERNAL
 * _grp_extract_section_info(): Extract the physical offset and size of each PE section.
 */
int _grp_extract_section_info(FILE *fpSample, char *nameSample, uint32_t *pIdBinary) {
    int      rc;
    size_t   nExptRead, nRealRead;
    uint32_t dwordReg, offsetPEHeader;
    uint16_t i, j, wordReg, countSection;
    BINARY   instBinary;
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
    
    /* Traverse each section header to retrieve the raw section offset and size. */
    for (i = 0 ; i < countSection ; i++) {
        instBinary.idBinary = *pIdBinary;
        instBinary.idxSection = i;
        instBinary.nameSample = nameSample;
        instBinary.hash = NULL;

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
        instBinary.sizeSection = dwordReg;

        /* Record the raw section offset. */
        dwordReg = 0;
        for (j = 1 ; j <= DATATYPE_SIZE_DWORD ; j++) {
            dwordReg <<= SHIFT_RANGE_8BIT;
            dwordReg += buf[SECTION_HEADER_OFF_RAW_OFFSET + DATATYPE_SIZE_DWORD - j] & 0xff;
        }
        instBinary.offsetSection = dwordReg;

        /* Insert the BINARY structure into array. */
        utarray_push_back(_arrayBinary, &instBinary);
        
        *pIdBinary = *pIdBinary + 1;        
    }
    
EXIT:
    return rc;
}

/**
 * !INTERNAL
 * _grp_generate_section_hash(): Generate the hash of each PE section.
 */
int _grp_generate_section_hash(FILE *fpSample, uint32_t idBgnBin, uint32_t idEndBin) {
    int      rc;
    uint32_t i, rawOffset, rawSize;
    size_t   nExptRead, nRealRead;
    char     *content;
    BINARY   *hBinary;    

    rc = 0;
    /* Traverse all the sections with non-zero size. */
    for (i = idBgnBin ; i < idEndBin ; i++) {
        hBinary = (BINARY*)utarray_eltptr(_arrayBinary, i);
        rawOffset = hBinary->offsetSection;
        rawSize = hBinary->sizeSection;
        
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
    
        hBinary->hash = (char*)malloc(sizeof(char) * FUZZY_MAX_RESULT);
        if (hBinary->hash == NULL) {
            Spew0("Error: Cannot allocate buffer for section hash.");
            rc = -1;
            goto FREE;
        }
        
        /* Apply the ssdeep library to generate section hash. */
        fuzzy_hash_buf(content, rawSize, hBinary->hash);
    FREE:
        free(content);    
    }

EXIT:
    return rc;
}
