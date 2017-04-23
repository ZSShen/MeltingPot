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


#ifndef _SLICE_H_
#define _SLICE_H_


#include <stdint.h>
#include <glib.h>
#include "data.h"


/* The exported interface to interact with this plugin. */
/* The function pointer types. */
typedef int8_t (*func_SlcInit) ();
typedef int8_t (*func_SlcDeinit) ();
typedef int8_t (*func_SlcGetFileSlice) (char*, uint16_t, GPtrArray**);

/* The integrated structure to store exported functions. */
typedef struct _PLUGIN_SLICE_T {
    void *hdle_Lib;
    func_SlcInit Init;
    func_SlcDeinit Deinit;
    func_SlcGetFileSlice GetFileSlice;
} PLUGIN_SLICE;

/* The function name symbols. */
#define SYM_SLC_INIT                "SlcInit"
#define SYM_SLC_DEINIT              "SlcDeinit"
#define SYM_SLC_GET_FILE_SLICE      "SlcGetFileSlice"


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
 * of SLICE structures to locate these slices.
 * 
 * @param szPathFile    The absoluate pathname of the given file.
 * @param usSizeSlc     The size of the to be created slice.
 * @param p_aSlc        The pointer to the GPtrArray which is to be filled
 *                      with SLICE structures.
 *
 * @return status code
 */
int8_t
SlcGetFileSlice(char *szPathFile, uint16_t usSizeSlc, GPtrArray **p_aSlc);


#endif
