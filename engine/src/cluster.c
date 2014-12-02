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


CONFIG *p_Conf;


int8_t
ClsInit(char *szPathCfg)
{
    int8_t cRtnCode = CLS_SUCCESS;
    
    config_t cfg;
    config_init(&cfg);

    int8_t cStat = config_read_file(&cfg, szPathCfg);
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_FILE_IO, EXIT1, "Error: %s.", config_error_text(&cfg));
    }

    p_Conf = (CONFIG*)malloc(sizeof(CONFIG));
    if (p_Conf == NULL) {
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT1, "Error: %s.", FAIL_MEM_ALLOC_CONF);
    }

    cStat = config_lookup_int(&cfg, C_COUNT_THREAD, (int*)&(p_Conf->ucCntThrd));    
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT1, "Error: %s missed.", C_COUNT_THREAD);        
    }
    cStat = config_lookup_int(&cfg, C_THRESHOLD_SIMILARITY, (int*)&(p_Conf->ucCntThrd));    
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT1, "Error: %s missed.", C_THRESHOLD_SIMILARITY);        
    }
    cStat = config_lookup_int(&cfg, C_COUNT_HEX_BLOCK, (int*)&(p_Conf->ucCntBlk));
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT1, "Error: %s missed.", C_COUNT_HEX_BLOCK);
    }
    cStat = config_lookup_int(&cfg, C_SIZE_HEX_BLOCK, (int*)&(p_Conf->ucSizeBlk));
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT1, "Error: %s missed.", C_SIZE_HEX_BLOCK);
    }
    cStat = config_lookup_int(&cfg, C_SIZE_SLICE, (int*)&(p_Conf->ucSizeSlc));
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT1, "Error: %s missed.", C_SIZE_SLICE);
    }
    cStat = config_lookup_string(&cfg, C_PATH_ROOT_INPUT,
                                (const char**)&(p_Conf->szPathRootIn));
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT1, "Error: %s missed.", C_PATH_ROOT_INPUT);
    }
    cStat = config_lookup_string(&cfg, C_PATH_ROOT_OUTPUT,
                                (const char**)&(p_Conf->szPathRootOut));
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT1, "Error: %s missed.", C_PATH_ROOT_OUTPUT);
    }
    cStat = config_lookup_string(&cfg, C_PATH_PLUGIN_SLICE,
                                (const char**)&(p_Conf->szPathPluginSlc));
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT1, "Error: %s missed.", C_PATH_PLUGIN_SLICE);
    }
    cStat = config_lookup_string(&cfg, C_PATH_PLUGIN_SIMILARITY,
                                (const char**)&(p_Conf->szPathPluginSim));
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT1, "Error: %s missed.", C_PATH_PLUGIN_SIMILARITY);
    }

EXIT1:
    config_destroy(&cfg);
    return cRtnCode;    
}


int8_t
ClsDeinit()
{
    if (p_Conf != NULL) {
        free(p_Conf);
    }

    return CLS_SUCCESS;
}

int8_t
ClsRunTask()
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
        EXIT1(CLS_FAIL_OPT_PARSE, EXIT1, "Error: %s.", FAIL_OPT_PARSE_CONF);
    }

    /* Initialize the clustering engine with user specified configurations. */
    int8_t cStat = ClsInit(szPathCfg);
    if (cStat != CLS_SUCCESS) {
        goto EXIT1;
    }

    /* Launch binary slice correlation and pattern generation. */
    cStat = ClsRunTask();

    /* Release the resources allocated by engine. */
    ClsDeinit();    
EXIT1:
    return cRtnCode;
}
