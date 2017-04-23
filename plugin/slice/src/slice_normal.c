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
#include <errno.h>
#include "except.h"
#include "spew.h"
#include "slice.h"
#include "slice_normal.h"


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

    FILE *fp = fopen(szPathFile, "r");
    if (!fp)
        EXIT1(FAIL_FILE_IO, EXIT, "Error: %s.", strerror(errno));

    *p_aSlc = NULL;
    *p_aSlc = g_ptr_array_new();
    if (!*p_aSlc)
        EXIT1(FAIL_MEM_ALLOC, CLOSE, "Error: %s.", strerror(errno));

    fseek(fp, 0, SEEK_END);
    uint64_t ulSizeFile = ftell(fp);
    uint64_t ulOfst = 0;
    while (ulOfst < ulSizeFile) {
        SLICE *p_Slc = (SLICE*)malloc(sizeof(SLICE));
        if (!p_Slc)
            EXIT1(FAIL_MEM_ALLOC, CLOSE, "Error: %s.", strerror(errno));

        p_Slc->iIdSec = -1;
        p_Slc->ulOfstAbs = ulOfst;
        p_Slc->ulOfstRel = ulOfst;
        p_Slc->szPathFile = szPathFile;
        g_ptr_array_add(*p_aSlc, (gpointer)p_Slc);
        ulOfst += usSizeSlc;
    }

CLOSE:
    if (fp)
        fclose(fp);

EXIT:
    return cRtnCode;
}
