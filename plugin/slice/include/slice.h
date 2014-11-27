#ifndef _SLICE_H_
#define _SLICE_H_


#include <stdint.h>
#include <glib.h>


#define FAIL_MEM_ALLOC_SLICE_ARRAY      "Fail to allocate array for SLICEs"
#define FAIL_MEM_ALLOC_SLICE            "Fail to allocate SLICE structure"


enum {
    RC_SUCCESS = 0,
    RC_FAIL_FILE_IO = -1,
    RC_FAIL_MEM_ALLOC = -2,
    RC_INVALID_FILE_FORMAT = -3
};


/* Slice is a file block containing designated number of bytes. */
/* This ds records information to locate a slice. */
typedef struct _SLICE_T {
    int32_t iSecId;         /* The section id of the host file. (For certain file type) */
    uint64_t ulOfstAbs;     /* The absolate offset of the host file. */
    uint64_t ulOfstRel;     /* The relative offset to the section starting address.*/
    union {
        uint64_t uiIdSlc;   /* The logic id for memorization. */
        uint64_t uiIdGrp;   /* The group id (after correlation) this slice belonging to.*/
    };
    char *szPathFile;       /* The aboslute path of the host file. */
} SLICE;


/**
 * This function initializes the file slicing plugin.
 *
 * @return status code
 */
int8_t
SlcInit();


/**
 * This function releases the resources allocated by file slicing plugin.
 *
 * @return status code
 */
int8_t
SlcDeinit();


/**
 * This function splits the given file into slices and returns an array
 * of addressing information to locate these slices.
 * 
 * @param szPathFile    The absoluate pathname of the given file.
 * @param usSizeSlc     The size of the to be created slice.
 * @param p_aSlc        The pointer to a array which is to be filled
 *                      with file slice addressing information.
 *
 * @return status code
 */
int8_t
SlcGetFileSlice(char *szPathFile, uint16_t usSizeSlc, GPtrArray **p_aSlc);


/**
 * This function hints the Glib to deallocate array elements.
 *
 * @param gp_Slc       The pointer to the to be deallocated element.
 */
void
SlcFreeSliceArray(gpointer gp_Slc);


#endif
