#ifndef _CLUSTER_H_
#define _CLUSTER_H_


#include <stdint.h>
#include <stdbool.h>


/* The messages for debugging. */
#define FAIL_OPT_PARSE_CONF      "Fail to parse \"conf\" option"
#define FAIL_NO_SAMPLE           "No samples in the given folder"
#define SLICE_GENERATION_FAIL    "[Slice Generation] failed!"
#define SLICE_GENERATION_SUCC    "[Slice Generation] succeed!"
#define SLICE_CORRELATION_FAIL   "[Slice Correlation] failed!"
#define SLICE_CORRELATION_SUCC   "[Slice Correlation] succeed!"
#define PATTERN_GENERATION_FAIL  "[Pattern Generation] failed!"
#define PATTERN_GENERATION_SUCC  "[Pattern Generation] succeed!"
#define YARA_OUTPUT_FAIL         "[YARA Output] failed!"
#define YARA_OUTPUT_SUCC         "[YARA Output] succeed!"


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


enum {
    CLS_SUCCESS = 0,
    CLS_FAIL_FILE_IO = -1,
    CLS_FAIL_MEM_ALLOC = -2,
    CLS_FAIL_OPT_PARSE = -3,
    CLS_FAIL_CONF_PARSE = -4,
    CLS_FAIL_PLUGIN_RESOLVE = -5,
    CLS_FAIL_PLUGIN_INTERACT = -6,
    CLS_FAIL_PROCESS = -7
};


/* This ds records the paramters to control the clustering process.
   Note the reason that we do not follow the definition of standard integer 
   is due to the type definition of libconfig. */
typedef struct _CONFIG_T {
    int ucCntThrd;          /* The number of threads used for parallel processing. */
    int ucScoreSim;         /* The threshold to group similar binary slices. */
    int ucCntBlk;           /* The number of hex byte blocks in pattern. */
    int ucSizeBlk;          /* The size of the hex byte block. */
    int ucSizeTruncGrp;     /* The threshold to truncate groups with size less than it. */
    int ucRatNoise;         /* The ratio of meaningless bytes in one hex block. */
    int ucRatWild;          /* The ratio of wildcard characters in one hex block. */
    int ucIoBand;           /* The number of files a thread can simultaneously open. */
    int usSizeSlc;          /* The size of the binary slice extracted from file
                               which is the basic unit of clustering process. */
    char *szPathRootIn;     /* The root pathname of the input sample set. */
    char *szPathRootOut;    /* The root pathname of the output patterns. */
    char *szPathPluginSlc;  /* The pathname of the plugin which is to be used for 
                               file slicing. */
    char *szPathPluginSim;  /* The pathname of the plugin which is to be used for 
                               similarity computation. */
} CONFIG;


/**
 * This function initializes the clustering engine with the configuration
 * specified by user.
 *
 * @param szPathCfg     The pathname of the task configuration.
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
 * This function launches the major clustering workflow.
 *
 * @return status code
 */
int8_t
ClsRunTask();


#endif
