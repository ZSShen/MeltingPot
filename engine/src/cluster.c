#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <libconfig.h>
#include "spew.h"
#include "cluster.h"
#include "slice.h"
#include "similarity.h"


int8_t
ClsInit(char *szPathCfg)
{
    int8_t cRtnCode = CLS_SUCCESS;
    
    config_t cfgCluster;
    config_init(&cfgCluster);
    
    
EXIT:
    config_destroy(&cfgCluster);
    return cRtnCode;    
}


int8_t
ClsDeinit()
{
    return CLS_SUCCESS;
}


int8_t
main(int argc, char **argv, char **envp)
{
    int8_t cRtnCode = CLS_SUCCESS;

    /* Handle comman line option. */
    static struct option options[] = {
        {OPT_LONG_HELP, no_argument, 0, OPT_HELP},
        {OPT_LONG_PATH_CONF, required_argument, 0, OPT_PATH_CONF},
    };

    char szOrder[BUF_SIZE_OPT];
    memset(szOrder, 0, sizeof(char) * BUF_SIZE_OPT);
    sprintf(szOrder, "%c%c:", OPT_HELP, OPT_PATH_CONF);    
    
    int32_t iIdxOpt = 0, iOpt;
    char *szPathCfg = NULL;
    while ((iOpt = getopt_long(argc, argv, szOrder, options, &iIdxOpt)) != -1) {
        switch (iOpt) {
            case OPT_PATH_CONF: {
                szPathCfg = optarg;
                break;
            }
            case OPT_HELP: {
                break;
            }
        }
    }
    if (szPathCfg == NULL) {
        EXIT1(CLS_FAIL_OPT_PARSE, EXIT, "Error: %s.", FAIL_OPT_PARSE_CONF);
    }

EXIT:
    return cRtnCode;
}
