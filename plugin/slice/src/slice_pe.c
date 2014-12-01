#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "spew.h"
#include "slice.h"
#include "slice_pe.h"


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

    /* Check the MZ header. */
    char buf[BUF_SIZE_BINARY];
    size_t nReadExpt = MZ_HEADER_SIZE;
    size_t nReadReal = fread(buf, sizeof(char), nReadExpt, fp);    
    if (nReadExpt != nReadReal) {
        EXIT1(SLC_RC_FAIL_FILE_IO, CLOSE, "Error: %s.", strerror(errno));
    } else if ((buf[0] != 'M') || (buf[1] != 'Z')) {
        EXIT1(SLC_RC_INVALID_FILE_FORMAT, CLOSE, "Error: %s.", INVALID_MZ_HEADER);
    }

    /* Resolve the starting address of PE header and move to it. */
    uint32_t uiReg = 0;
    uint16_t usIterFst;
    for (usIterFst = 1 ; usIterFst <= DATATYPE_SIZE_DWORD ; usIterFst++) {
        uiReg <<= SHIFT_RANGE_8BIT;
        uiReg += buf[MZ_HEADER_OFF_PE_HEADER_OFFSET + DATATYPE_SIZE_DWORD - usIterFst] & 0xff;
    }
    uint32_t uiOfstPEHeader = uiReg;
    uint32_t uiStat = fseek(fp, uiOfstPEHeader, SEEK_SET);
    if (uiStat != 0) {
        EXIT1(SLC_RC_FAIL_FILE_IO, CLOSE, "Error: %s.", strerror(errno));
    }

    /* Check the PE header. */
    nReadExpt = PE_HEADER_SIZE;
    nReadReal = fread(buf, sizeof(char), nReadExpt, fp);
    if (nReadExpt != nReadReal) {
        EXIT1(SLC_RC_FAIL_FILE_IO, CLOSE, "Error: %s.", strerror(errno));
    } else if ((buf[0] != 'P') || (buf[1] != 'E')) {
        EXIT1(SLC_RC_INVALID_FILE_FORMAT, CLOSE, "Error: %s.", INVALID_PE_HEADER);
    }

    /* Resolve the number of sections. */
    uint16_t usReg = 0;
    for (usIterFst = 1 ; usIterFst <= DATATYPE_SIZE_WORD ; usIterFst++) {
        usReg <<= SHIFT_RANGE_8BIT;
        usReg += buf[PE_HEADER_OFF_NUMBER_OF_SECTION + DATATYPE_SIZE_WORD - usIterFst] & 0xff;
    }
    uint16_t usCountSec = usReg;

    /* Resolve the size of optional header. */
    usReg = 0;
    for (usIterFst = 1 ; usIterFst <= DATATYPE_SIZE_WORD ; usIterFst++) {
        usReg <<= SHIFT_RANGE_8BIT;
        usReg += buf[PE_HEADER_OFF_SIZE_OF_OPT_HEADER + DATATYPE_SIZE_WORD - usIterFst] & 0xff;
    }
        
    /* Move to the starting address of the section headers. */
    uiStat = fseek(fp, (uiOfstPEHeader + PE_HEADER_SIZE + usReg), SEEK_SET);
    if (uiStat != 0) {
        EXIT1(SLC_RC_FAIL_FILE_IO, CLOSE, "Error: %s.", strerror(errno));
    }

    /* Traverse each section header to get the physical section offset and size. */
    for (usIterFst = 0 ; usIterFst < usCountSec ; usIterFst++) {
        nReadExpt = SECTION_HEADER_PER_ENTRY_SIZE;
        nReadReal = fread(buf, sizeof(char), nReadExpt, fp);
        if (nReadExpt != nReadReal) {
            EXIT1(SLC_RC_FAIL_FILE_IO, CLOSE, "Error: %s.", strerror(errno));
        }

        uiReg = 0;
        uint16_t usIterSnd;
        for (usIterSnd = 1 ; usIterSnd <= DATATYPE_SIZE_DWORD ; usIterSnd++) {
            uiReg <<= SHIFT_RANGE_8BIT;
            uiReg += buf[SECTION_HEADER_OFF_RAW_SIZE + DATATYPE_SIZE_DWORD - usIterSnd] & 0xff;
        }
        if (uiReg == 0) {
            continue;
        }
        int32_t iSizeSec = uiReg;
        uiReg = 0;
        for (usIterSnd = 1 ; usIterSnd <= DATATYPE_SIZE_DWORD ; usIterSnd++) {
            uiReg <<= SHIFT_RANGE_8BIT;
            uiReg += buf[SECTION_HEADER_OFF_RAW_OFFSET + DATATYPE_SIZE_DWORD - usIterSnd] & 0xff;
        }
        uint32_t uiOfstSec = uiReg;
        uint32_t uiOfstRel = 0;
        while (iSizeSec > 0) {
            SLICE *p_Slc = (SLICE*)malloc(sizeof(SLICE));
            if (!p_Slc) {
                EXIT1(SLC_RC_FAIL_MEM_ALLOC, CLOSE, "Error: %s.", FAIL_MEM_ALLOC_SLICE);
            }
            p_Slc->iIdSec = usIterFst;
            p_Slc->ulOfstAbs = uiOfstSec;
            p_Slc->ulOfstRel = uiOfstRel;
            p_Slc->usSize = (usSizeSlc < iSizeSec)? usSizeSlc : iSizeSec;
            p_Slc->szPathFile = szPathFile;
            g_ptr_array_add(*p_aSlc, (gpointer)p_Slc);
            iSizeSec -= usSizeSlc;
            uiOfstSec += usSizeSlc;
            uiOfstRel += usSizeSlc;
        }
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
