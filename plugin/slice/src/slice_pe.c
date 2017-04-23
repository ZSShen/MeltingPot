/**
 *   The MIT License (MIT)
 *   Copyright (C) 2014-2017 ZongXian Shen <andy.zsshen@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include "except.h"
#include "spew.h"
#include "slice.h"
#include "slice_pe.h"


int8_t
SlcInit()
{
    return SUCCESS;
}


int8_t
SlcDeinit()
{
    return SUCCESS;
}


int8_t
SlcGetFileSlice(char *szPathFile, uint16_t usSizeSlc, GPtrArray **p_aSlc)
{
    int8_t cRtnCode = SUCCESS;

    char *szBin = (char*)malloc(sizeof(char) * usSizeSlc);
    if (!szBin)
        EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    *p_aSlc = NULL;
    *p_aSlc = g_ptr_array_new();
    if (!*p_aSlc)
        EXIT1(FAIL_MEM_ALLOC, FREE, "Error: %s.", strerror(errno));

    FILE *fp = fopen(szPathFile, "r");
    if (!fp) {
        g_ptr_array_free(*p_aSlc, true);
        *p_aSlc = NULL;
        EXIT1(FAIL_FILE_IO, FREE, "Error: %s.", strerror(errno));
    }

    /* Check the MZ header. */
    size_t nReadExpt = MZ_HEADER_SIZE;
    size_t nReadReal = fread(szBin, sizeof(char), nReadExpt, fp);    
    if (nReadExpt != nReadReal) {
        EXIT1(FAIL_FILE_IO, CLOSE, "Error: %s.", strerror(errno));
    } else if ((szBin[0] != 'M') || (szBin[1] != 'Z')) {
        EXIT1(FAIL_FILE_FORMAT, CLOSE, "Error: %s.", INVALID_MZ_HEADER);
    }

    /* Resolve the starting address of PE header and move to it. */
    uint32_t uiReg = 0;
    uint16_t usIterFst;
    for (usIterFst = 1 ; usIterFst <= DATATYPE_SIZE_DWORD ; usIterFst++) {
        uiReg <<= SHIFT_RANGE_8BIT;
        uiReg += szBin[MZ_HEADER_OFF_PE_HEADER_OFFSET + DATATYPE_SIZE_DWORD - usIterFst] & 0xff;
    }
    uint32_t uiOfstPEHeader = uiReg;
    int8_t cStat = fseek(fp, uiOfstPEHeader, SEEK_SET);
    if (cStat != 0)
        EXIT1(FAIL_FILE_IO, CLOSE, "Error: %s.", strerror(errno));

    /* Check the PE header. */
    nReadExpt = PE_HEADER_SIZE;
    nReadReal = fread(szBin, sizeof(char), nReadExpt, fp);
    if (nReadExpt != nReadReal) {
        EXIT1(FAIL_FILE_IO, CLOSE, "Error: %s.", strerror(errno));
    } else if ((szBin[0] != 'P') || (szBin[1] != 'E')) {
        EXIT1(FAIL_FILE_FORMAT, CLOSE, "Error: %s.", INVALID_PE_HEADER);
    }

    /* Resolve the number of sections. */
    uint16_t usReg = 0;
    for (usIterFst = 1 ; usIterFst <= DATATYPE_SIZE_WORD ; usIterFst++) {
        usReg <<= SHIFT_RANGE_8BIT;
        usReg += szBin[PE_HEADER_OFF_NUMBER_OF_SECTION + DATATYPE_SIZE_WORD - usIterFst] & 0xff;
    }
    uint16_t usCountSec = usReg;

    /* Resolve the size of optional header. */
    usReg = 0;
    for (usIterFst = 1 ; usIterFst <= DATATYPE_SIZE_WORD ; usIterFst++) {
        usReg <<= SHIFT_RANGE_8BIT;
        usReg += szBin[PE_HEADER_OFF_SIZE_OF_OPT_HEADER + DATATYPE_SIZE_WORD - usIterFst] & 0xff;
    }

    /* Determine the starting address of the section headers.*/
    uint32_t uiOfstSecHeader = uiOfstPEHeader + PE_HEADER_SIZE + usReg;

    /* Traverse each section header to get the physical section offset and size. */
    for (usIterFst = 0 ; usIterFst < usCountSec ; usIterFst++) {
        uint32_t uiOfstEntry = uiOfstSecHeader + usIterFst * SECTION_HEADER_PER_ENTRY_SIZE;
        cStat = fseek(fp, uiOfstEntry, SEEK_SET);
        if (cStat != 0)
            EXIT1(FAIL_FILE_IO, CLOSE, "Error: %s.", strerror(errno));

        nReadExpt = SECTION_HEADER_PER_ENTRY_SIZE;
        nReadReal = fread(szBin, sizeof(char), nReadExpt, fp);
        if (nReadExpt != nReadReal)
            EXIT1(FAIL_FILE_IO, CLOSE, "Error: %s.", strerror(errno));

        uiReg = 0;
        uint16_t usIterSnd;
        for (usIterSnd = 1 ; usIterSnd <= DATATYPE_SIZE_DWORD ; usIterSnd++) {
            uiReg <<= SHIFT_RANGE_8BIT;
            uiReg += szBin[SECTION_HEADER_OFF_RAW_SIZE + DATATYPE_SIZE_DWORD - usIterSnd] & 0xff;
        }
        if (uiReg == 0)
            continue;

        int32_t iSizeSec = uiReg;
        uiReg = 0;
        for (usIterSnd = 1 ; usIterSnd <= DATATYPE_SIZE_DWORD ; usIterSnd++) {
            uiReg <<= SHIFT_RANGE_8BIT;
            uiReg += szBin[SECTION_HEADER_OFF_RAW_OFFSET + DATATYPE_SIZE_DWORD - usIterSnd] & 0xff;
        }

        uint32_t uiOfstSec = uiReg;
        uint32_t uiOfstRel = 0;
        bool bLoop = true;
        while (bLoop) {
            size_t nReadExpt = (usSizeSlc < iSizeSec)? usSizeSlc : iSizeSec;
            size_t nReadReal = fread(szBin, sizeof(char), nReadExpt, fp);
            if (nReadExpt != nReadReal) {
                cStat = ferror(fp);
                if (cStat == 0)
                    bLoop = false;
                else
                    EXIT1(FAIL_FILE_IO, CLOSE, "Error: %s.", strerror(errno));    
            }

            SLICE *p_Slc = (SLICE*)malloc(sizeof(SLICE));
            if (!p_Slc)
                EXIT1(FAIL_MEM_ALLOC, CLOSE, "Error: %s.", strerror(errno));

            p_Slc->iIdSec = usIterFst;
            p_Slc->ulOfstAbs = uiOfstSec;
            p_Slc->ulOfstRel = uiOfstRel;
            p_Slc->usSize = nReadReal;
            p_Slc->szPathFile = szPathFile;
            g_ptr_array_add(*p_aSlc, (gpointer)p_Slc);

            iSizeSec -= nReadReal;
            uiOfstSec += nReadReal;
            uiOfstRel += nReadReal;
            if (iSizeSec <= 0)
                bLoop = false;
        }
    }    

CLOSE:
    if (fp)
        fclose(fp);
FREE:
    if (szBin)
        free(szBin);
EXIT:
    return cRtnCode;    
}
