#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include "cluster.h"
#include "entry.h"


int main(int argc, char **argv, char *envp) {
    int iRtnCode, iOpt, iIdxOpt;
    uint8_t ucParallelity, ucSimilarity, ucBlkCount, ucBlkSize, ucIoBandwidth;
    char *szPathInput, *szPathOutput;
    CONFIG *pCfg;
    CLUSTER *pCluster;
    char bufOrder[BUF_SIZE_SMALL];
    
    /* Craft the structure to store command line options. */    
    static struct option options[] = {
        {OPT_LONG_HELP           , no_argument      , 0, OPT_HELP           },
        {OPT_LONG_PATH_SAMPLE_SET, required_argument, 0, OPT_PATH_SAMPLE_SET},
        {OPT_LONG_PATH_PATTERN   , required_argument, 0, OPT_PATH_PATTERN   },
        {OPT_LONG_PARALLELITY    , required_argument, 0, OPT_PARALLELITY    },
        {OPT_LONG_BLK_COUNT      , required_argument, 0, OPT_BLK_COUNT      },
        {OPT_LONG_BLK_SIZE       , required_argument, 0, OPT_BLK_SIZE       },
        {OPT_LONG_SIMILARITY     , required_argument, 0, OPT_SIMILARITY     },
    };

    memset(bufOrder, 0, sizeof(char) * BUF_SIZE_SMALL);
    sprintf(bufOrder, "%c%c:%c:%c:%c:%c:%c:", OPT_HELP, OPT_PATH_SAMPLE_SET, OPT_PATH_PATTERN,
                                              OPT_PARALLELITY, OPT_BLK_COUNT, OPT_BLK_SIZE, 
                                              OPT_SIMILARITY);    
    iRtnCode = 0;
    szPathInput = szPathOutput = NULL;
    ucParallelity = ucSimilarity = ucBlkCount = ucBlkSize = ucIoBandwidth = 0;
    pCfg = NULL;
    pCluster = NULL;

    /* Get the command line options. */
    iIdxOpt = 0;
    while ((iOpt = getopt_long(argc, argv, bufOrder, options, &iIdxOpt)) != -1) {
        switch (iOpt) {
            case OPT_PATH_SAMPLE_SET: {
                szPathInput = optarg;
                break;
            }
            case OPT_PATH_PATTERN: {
                szPathOutput = optarg;
                break;
            }
            case OPT_PARALLELITY: {
                ucParallelity = atoi(optarg);
                break;
            }
            case OPT_SIMILARITY: {
                ucSimilarity = atoi(optarg);
                break;
            }
            case OPT_BLK_COUNT: {
                ucBlkCount = atoi(optarg);
                break;
            }
            case OPT_BLK_SIZE: {
                ucBlkSize = atoi(optarg);
                break;
            }
        }
    }

    /* Initialize the clustering library. */
    INIT_CLUSTER(pCluster);
    if (pCluster == NULL) {
        iRtnCode = -1;
        goto EXIT;
    }

    /* Set up the task configuration. */
    pCfg = (CONFIG*)malloc(sizeof(CONFIG));
    if (pCfg == NULL) {
        iRtnCode = -1;
        goto EXIT;
    }
    pCfg->ucParallelity = ucParallelity;
    pCfg->ucSimilarity = ucSimilarity;
    pCfg->ucBlkCount = ucBlkCount;
    pCfg->ucBlkSize = ucBlkSize;
    pCfg->ucIoBandwidth = HARDCODE_IO_BANDWIDTH;
    pCfg->szPathInput = szPathInput;
    pCfg->szPathOutput = szPathOutput;
    iRtnCode = pCluster->initCtx(pCluster, pCfg);
    if (iRtnCode != 0) {
        pCluster->showUsage(pCluster);
        goto EXIT;;   
    }

    /* Group the binaries of the given sample set. */
    iRtnCode = pCluster->generateGroup(pCluster);
    if (iRtnCode != 0) {
        goto EXIT;
    }

    /* Generate the patterns for the clustered PE sections. */
    iRtnCode = pCluster->generatePattern(pCluster);
    if (iRtnCode != 0) {
        goto EXIT;
    }

EXIT:
    /* Free the task configuration. */
    if (pCfg != NULL) {
        free(pCfg);
    }
    /* Free the clustering library. */
    if (pCluster != NULL) {
        pCluster->deinitCtx(pCluster);
        DEINIT_CLUSTER(pCluster);
    }

    return iRtnCode;
}

