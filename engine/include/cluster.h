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


#ifndef _CLUSTER_H_
#define _CLUSTER_H_


#include <stdint.h>
#include <stdbool.h>
#include "slice.h"
#include "similarity.h"
#include "format.h"


/* The messages for debugging. */
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RESET   "\x1b[0m"

#define FAIL_OPT_PARSE_CONF      "Fail to parse \"conf\" option"
#define FAIL_NO_SAMPLE           "No samples in the given folder"
#define SLC_GEN_FAIL    COLOR_RED"[Step 1/4] Slice Generation  **FAIL"COLOR_RESET
#define SLC_GEN_SUCC    COLOR_GREEN"[Step 1/4] Slice Generation  **SUCC"COLOR_RESET
#define SLC_CRL_FAIL    COLOR_RED"[Step 2/4] Slice Correlation  **FAIL"COLOR_RESET
#define SLC_CRL_SUCC    COLOR_GREEN"[Step 2/4] Slice Correlation  **SUCC"COLOR_RESET
#define PTN_GEN_FAIL    COLOR_RED"[Step 3/4] Pattern Crafting  **FAIL"COLOR_RESET
#define PTN_GEN_SUCC    COLOR_GREEN"[Step 3/4] Pattern Crafting  **SUCC"COLOR_RESET
#define PTN_OUT_FAIL    COLOR_RED"[Step 4/4] Pattern Output  **FAIL"COLOR_RESET
#define PTN_OUT_SUCC    COLOR_GREEN"[Step 4/4] Pattern Output  **SUCC"COLOR_RESET


/* The constants for command option parsing. */
#define BUF_SIZE_OPT          (64)    /* The size of option order buffer. */
#define OPT_LONG_HELP         "help"  /* The option for help message. */
#define OPT_LONG_PATH_CONF    "conf"  /* The abbreviated character. */
#define OPT_HELP              'h'     /* The option for the path of configuration file. */
#define OPT_PATH_CONF         'c'     /* The abbreviated character. */


/* The tags for each kind of configuration. */
#define C_COUNT_THREAD              "COUNT_THREAD"
#define C_THRESHOLD_SIMILARITY      "THRESHOLD_SIMILARITY"
#define C_COUNT_HEX_BLOCK           "COUNT_HEX_BLOCK"
#define C_SIZE_HEX_BLOCK            "SIZE_HEX_BLOCK"
#define C_SIZE_SLICE                "SIZE_SLICE"
#define C_RATIO_NOISE               "RATIO_NOISE"
#define C_RATIO_WILDCARD            "RATIO_WILDCARD"
#define C_IO_BANDWIDTH              "IO_BANDWIDTH"
#define C_SIZE_TRUNCATE_GROUP       "TRUNCATE_GROUP_SIZE_LESS_THAN"
#define C_PATH_ROOT_INPUT           "PATH_ROOT_INPUT"
#define C_PATH_ROOT_OUTPUT          "PATH_ROOT_OUTPUT"
#define C_PATH_PLUGIN_SLICE         "PATH_PLUGIN_SLICE"
#define C_PATH_PLUGIN_SIMILARITY    "PATH_PLUGIN_SIMILARITY"
#define C_PATH_PLUGIN_FORMAT        "PATH_PLUGIN_FORMAT"
#define C_FLAG_COMMENT              "FLAG_COMMENT"


/* The structure lists down the process control parameters. */
typedef struct _CONFIG_T {
    bool bComt;               /* The control flag for pattern comment. */
    uint8_t ucCntThrd;        /* The number of threads used for parallel processing. */
    uint8_t ucScoreSim;       /* The threshold to group similar binary slices. */
    uint8_t ucCntBlk;         /* The number of hex byte blocks in pattern. */
    uint8_t ucSizeBlk;        /* The size of the hex byte block. */
    uint8_t ucSizeTruncGrp;   /* The threshold to truncate groups with size less than it. */
    uint8_t ucRatNoise;       /* The ratio of meaningless bytes in one hex block. */
    uint8_t ucRatWild;        /* The ratio of wildcard characters in one hex block. */
    uint8_t ucIoBand;         /* The number of files a thread can simultaneously open. */
    uint16_t usSizeSlc;       /* The size of the binary slice extracted from file
                                 which is the basic unit of clustering process. */
    char *szPathRootIn;       /* The root pathname of the input sample set. */
    char *szPathRootOut;      /* The root pathname of the output patterns. */
    char *szPathPluginSlc;    /* The pathname of the plugin which is for file slicing. */
    char *szPathPluginSim;    /* The pathname of the plugin which is for similarity computation. */
    char *szPathPluginFmt;    /* The pathname of the plugin which is for pattern generation. */
} CONFIG;

/* The structure recording the entire process context. */
typedef struct _CONTEXT_T {
    CONFIG *p_Conf;
    MELT_POT *p_Pot;
    PLUGIN_SLICE *plgSlc;
    PLUGIN_SIMILARITY *plgSim;
    PLUGIN_FORMAT *plgFmt;
} CONTEXT;


/**
 * This function initializes the clustering engine with the configuration
 * specified by user.
 *
 * @param szPathCfg     The pathname of the configuration file.
 *
 * @return status code
 */
int8_t
ClsInit(char *szPathCfg);


/**
 * This function releases the resources allocated by clustering engine.
 *
 * @return status code
 */
int8_t
ClsDeinit();


/**
 * This function launches the main clustering workflow.
 *
 * @return status code
 */
int8_t
ClsRunTask();


#endif
