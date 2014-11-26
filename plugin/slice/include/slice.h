#ifndef _SLICE_H_
#define _SLICE_H_


#include <stdint.h>
#include <glib.h>


/* Slice is a file block containing designated number of bytes. */
/* This ds records information to locate a slice. */
typedef struct _SLICE_T {
    int32_t iSectId;        /* The section id of the host file. (For certain file type) */
    uint32_t uiOfstAbs;     /* The absolate offset of the host file. */
    uint32_t uiOfstRel;     /* The relative offset to the section starting address.*/
    union {
        uint32_t uiIdSlc;   /* The logic id for memorization. */
        uint32_t uiIdGrp;   /* The group id (after correlation) this slice belonging to.*/
    };
    char *szPathFile;       /* The aboslute path of the host file. */
} SLICE;


int8_t SlcInit(); 


int8_t SlcDeinit();


int8_t SlcGetFileSlice();


void SlcFreeSliceArray(gpointer gp_Slc);


#endif
