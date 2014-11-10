#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "cluster.h"
#include "entry.h"


/* Print the program usage message. */
void print_usage();


int main(int argc, char **argv, char *envp) {
    int     rc, opt, idxOpt, nParallelity, nSimilarity, nBlkCount, nBlkSize;
    char    *pathRoot, *pathPat;
    CONFIG  *hConfig;
    CLUSTER *hCluster;
    char    bufOrder[BUF_SIZE_SMALL];
    
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
    rc = 0;
    pathRoot = NULL;
    nParallelity = nSimilarity = nBlkCount = nBlkSize = -1;

    /* Get the command line options. */
    idxOpt = 0;
    while ((opt = getopt_long(argc, argv, bufOrder, options, &idxOpt)) != -1) {
        switch (opt) {
            case OPT_PATH_SAMPLE_SET: {
                pathRoot = optarg;
                break;
            }
            case OPT_PATH_PATTERN: {
                pathPat = optarg;
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
                EARLY_RETURN(rc);
            }
        }
    }

    /* Check the configuration parameters. */
    if ((pathRoot == NULL) || (pathPat == NULL)) {
        EARLY_RETURN(rc);
    }
    if ((nParallelity <= 0) || (nSimilarity <= 0) ||
        (nBlkCount <= 0) || (nBlkSize <= 0)) {
        EARLY_RETURN(rc);
    }

    /* Initialize the clustering library. */
    INIT_CLUSTER(hCluster);
    if (hCluster == NULL) {
        EARLY_RETURN(rc);
    }

    /* Set up the context. */
    hConfig = (CONFIG*)malloc(sizeof(CONFIG));
    if (hConfig == NULL) {
        rc = -1;
        goto DEINIT;
    }
    hConfig->cfgParallelity = nParallelity;
    hConfig->cfgSimilarity = nSimilarity;
    hConfig->cfgBlkCount = nBlkCount;
    hConfig->cfgBlkSize = nBlkSize;
    hConfig->cfgPathRoot = pathRoot;
    hConfig->cfgPathPat = pathPat;
    rc = hCluster->init_ctx(hCluster, hConfig);
    if (rc != 0) {
        rc = -1;
        goto DEINIT;   
    }

    /* Group the binaries of the given sample set. */
    rc = hCluster->generate_group(hCluster);
    if (rc != 0) {
        rc = -1;
        goto FREECTX;
    }

FREECTX:
    /* Free the context. */
    hCluster->deinit_ctx(hCluster);

DEINIT:
    if (hConfig != NULL) {
        free(hConfig);
    }
    /* Deinitialize the clustering library. */
    DEINIT_CLUSTER(hCluster);

EXIT:
    return rc;
}


void print_usage() {

    const char *strMsg = "Usage: entry --input path_in --output path_out --parallelity num_p --similarity num_t"
                         " --blkcount num_c --blksize num_s\n"
                         "       entry -i      path_in -o       path_out -p            num_p -t           num_t"
                         " -c         num_c -s        num_s\n\n"
                         "Example: entry --input /path/set --output /path/pattern --parallelity 2 --similarity 80"
                         " --blkcount 4 --blksize 16\n"
                         "         entry -i      /path/set -o       /path/pattern -p            2 -t           80"
                         " -c         4 -s        16\n\n";
    printf("%s", strMsg);
    return;
}
