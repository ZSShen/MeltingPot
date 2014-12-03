#ifndef _CLUSTER_H_
#define _CLUSTER_H_


#include <stdint.h>


/* The error messages for debugging. */
#define FAIL_OPT_PARSE_CONF      "Fail to parse \"conf\" option"

/* The constants for command option parsing. */
#define BUF_SIZE_OPT          64      /* The size of option order buffer. */
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
    CLS_FAIL_PLUGIN_INTERACT = -6
};


/* This ds records the paramters to control the clustering process. */
typedef struct _CONFIG_T {
    uint8_t ucCntThrd;      /* The number of threads used for parallel processing. */
    uint8_t ucScoreSim;     /* The threshold to group similar binary slices. */
    uint8_t ucCntBlk;       /* The number of hex byte blocks in pattern. */
    uint8_t ucSizeBlk;      /* The size of the hex byte block. */    
    uint8_t ucSizeSlc;      /* The size of the binary slice extracted from file
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
