#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "cluster.h"
#include "entry.h"


int main(int argc, char **argv, char *envp) {
    int         rc, opt, idxOpt, nParallelity, nSimilarity, nBlkCount, nBlkSize;
    const char  *pathRoot;
    char        bufOrder[BUF_SIZE_SMALL];
    
    /* Craft the structure to store command line options. */    
    static struct option options[] = {
        {OPT_LONG_HELP           , no_argument      , 0, OPT_HELP },
        {OPT_LONG_PATH_SAMPLE_SET, required_argument, 0, OPT_PATH_SAMPLE_SET},
        {OPT_LONG_PARALLELITY    , required_argument, 0, OPT_PARALLELITY},
        {OPT_LONG_BLK_COUNT      , required_argument, 0, OPT_BLK_COUNT},
        {OPT_LONG_BLK_SIZE       , required_argument, 0, OPT_BLK_SIZE},
        {OPT_LONG_SIMILARITY     , required_argument, 0, OPT_SIMILARITY},
    };

    memset(bufOrder, 0, sizeof(char) * BUF_SIZE_SMALL);
    sprintf(bufOrder, "%c%c:%c:%c:%c:%c:", OPT_HELP, OPT_PATH_SAMPLE_SET, OPT_PARALLELITY, 
                                           OPT_BLK_COUNT, OPT_BLK_SIZE, OPT_SIMILARITY);    
    rc = 0;
    pathRoot = NULL;
    
    /* Get the command line options. */
    idxOpt = 0;
    while ((opt = getopt_long(argc, argv, bufOrder, options, &idxOpt)) != -1) {
        switch (opt) {
            case OPT_PATH_SAMPLE_SET: {
                pathRoot = optarg;
                break;
            }
            case OPT_PARALLELITY: {
                nParallelity = atoi(optarg);
                break;
            }
            case OPT_SIMILARITY: {
                nSimilarity = atoi(optarg);
                break;
            }
            case OPT_BLK_COUNT: {
                nBlkCount = atoi(optarg);
                break;
            }
            case OPT_BLK_SIZE: {
                nBlkSize = atoi(optarg);
                break;
            }
            default: {
                rc = -1;
                goto EXIT;            
            }
        }
    }

    /* Check the configuration parameters. */
    if (pathRoot == NULL) {
        rc = -1;
        goto EXIT;
    }

EXIT:
    return rc;
}
