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


#ifndef _FORMAT_H_
#define _FORMAT_H_


#include <stdbool.h>
#include <stdint.h>
#include <glib.h>
#include "data.h"


/* The constants helping for YARA pattern creation. */
#define WILD_CARD_MARK          (0x100) /* The marker for wildcard character. */
#define EXTENSION_MASK          (0xff)  /* The mask used to extend char type to short. */

#define SIZE_INDENT_MAX         (64)    /* The maxumum indentation length. */
#define SIZE_HEX_LINE           (16)    /* The maximum number of bytes in a single line. */

#define PREFIX_PATTERN          "AUTO"  /* The prefix for pattern name. */
#define PREFIX_HEX_STRING       "SEQ"   /* The prefix for hex string name. */
#define SPACE_SUBS_TAB          "    "  /* The spaces substituting a tab. */
#define COMMENT_CONTRIBUTE      "Contributed By"
#define COMMENT_RELATIVE_OFFSET "Relative Offset"


/* The exported interface to interact with this plugin. */
/* The function pointer type. */
typedef int8_t (*func_FmtInit) ();
typedef int8_t (*func_FmtDeinit) ();
typedef int8_t (*func_FmtPrint) (char*, uint64_t, GROUP*, bool);

/* The integrated structure to store exported functions. */
typedef struct _PLUGIN_FORMAT_T {
    void *hdle_Lib;
    func_FmtInit Init;
    func_FmtDeinit Deinit;
    func_FmtPrint Print;
} PLUGIN_FORMAT;

/* The function name symbols. */
#define SYM_FMT_INIT                "FmtInit"
#define SYM_FMT_DEINIT              "FmtDeinit"
#define SYM_FMT_PRINT               "FmtPrint"


/* The structure helping to generate condition section and comment block. */
typedef struct _TRAV_T {
    bool bDeclare;
    uint8_t ucIdxBlk;
    uint8_t ucCntBlk;
    uint64_t ulIdxCond;
    uint64_t ulCntCond;
    PATTERN_TEXT *p_Text;
} TRAV;


/**
 * This function initializes the pattern formatter plugin.
 *
 * @return status code
 */
int8_t
FmtInit();


/**
 * This function releases the resources allocated by pattern formatter plugin.
 *
 * @return status code
 */
int8_t
FmtDeinit();


/**
 * This function formats the common byte sequences of a given group into
 * the YARA format and outputs the pattern file.
 * 
 * @param szPathRoot        The root pathname of the output pattern folder.
 * @param ulIdxGrp          The group index.
 * @param p_Grp             The pointer to the GROUP structure.
 * @param bComt             The control flag for pattern comment.
 * 
 * @return status code.
 */
int8_t
FmtPrint(char *szPathRoot, uint64_t ulIdxGrp, GROUP *p_Grp, bool bComt);


#endif
