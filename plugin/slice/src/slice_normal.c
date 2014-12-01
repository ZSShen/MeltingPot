#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "spew.h"
#include "slice.h"
#include "slice_normal.h"


int8_t
SlcInit()
{
    return SLC_RC_SUCCESS;
}


int8_t
SlcDeinit()
{
    return SLC_RC_SUCCESS;
}


int8_t
SlcGetFileSlice(char *szPathFile, uint16_t usSizeSlc, GPtrArray **p_aSlc)
{
    int8_t cRtnCode = SLC_RC_SUCCESS;

    FILE *fp = fopen(szPathFile, "r");
    if (!fp) {
        EXIT1(SLC_RC_FAIL_FILE_IO, RTN, "Error: %s.", strerror(errno));
    }

    *p_aSlc = NULL;
    *p_aSlc = g_ptr_array_new_with_free_func(SlcFreeSliceArray);
    if (!*p_aSlc) {
        EXIT1(SLC_RC_FAIL_MEM_ALLOC, CLOSE, "Error: %s.", FAIL_MEM_ALLOC_SLICE_ARRAY);
    }

    fseek(fp, 0, SEEK_END);
    uint64_t ulSizeFile = ftell(fp);
    uint64_t ulOfst = 0;
    while (ulOfst < ulSizeFile) {
        SLICE *p_Slc = (SLICE*)malloc(sizeof(SLICE));
        if (!p_Slc) {
            EXIT1(SLC_RC_FAIL_MEM_ALLOC, CLOSE, "Error: %s.", FAIL_MEM_ALLOC_SLICE);
        }
        p_Slc->iIdSec = -1;
        p_Slc->ulOfstAbs = ulOfst;
        p_Slc->ulOfstRel = ulOfst;
        p_Slc->szPathFile = szPathFile;
        g_ptr_array_add(*p_aSlc, (gpointer)p_Slc);
        ulOfst += usSizeSlc;
    }

CLOSE:
    if (fp) {
        fclose(fp);
    }
RTN:
    return cRtnCode;
}


void
SlcFreeSliceArray(gpointer gp_Slc)
{
    SLICE *p_Slc = (SLICE*)gp_Slc;
    if (p_Slc) {
        free(p_Slc);
    }

    return;
}
