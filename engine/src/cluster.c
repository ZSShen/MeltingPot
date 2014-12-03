#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <libconfig.h>
#include <dlfcn.h>
#include "spew.h"
#include "data.h"
#include "cluster.h"
#include "correlate.h"
#include "pattern.h"
#include "slice.h"
#include "similarity.h"


config_t cfg;
CONFIG *p_Conf;
PLUGIN_SLICE *plg_Slc;
PLUGIN_SIMILARITY *plg_Sim;
MELT_POT *p_Pot;


int8_t
ClsInit(char *szPathCfg)
{
    int8_t cRtnCode = CLS_SUCCESS;
    
    config_init(&cfg);
    p_Conf = NULL;
    plg_Slc = NULL;
    plg_Sim = NULL;

    int8_t cStat = config_read_file(&cfg, szPathCfg);
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_FILE_IO, EXIT, "Error: %s.", config_error_text(&cfg));
    }
    p_Conf = (CONFIG*)malloc(sizeof(CONFIG));
    if (p_Conf == NULL) {
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));
    }

    /* Resolve the user specified configuration. */
    cStat = config_lookup_int(&cfg, C_COUNT_THREAD, (int*)&(p_Conf->ucCntThrd));    
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_COUNT_THREAD);        
    }
    cStat = config_lookup_int(&cfg, C_THRESHOLD_SIMILARITY, (int*)&(p_Conf->ucCntThrd));    
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_THRESHOLD_SIMILARITY);        
    }
    cStat = config_lookup_int(&cfg, C_COUNT_HEX_BLOCK, (int*)&(p_Conf->ucCntBlk));
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_COUNT_HEX_BLOCK);
    }
    cStat = config_lookup_int(&cfg, C_SIZE_HEX_BLOCK, (int*)&(p_Conf->ucSizeBlk));
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_SIZE_HEX_BLOCK);
    }
    cStat = config_lookup_int(&cfg, C_SIZE_SLICE, (int*)&(p_Conf->usSizeSlc));
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_SIZE_SLICE);
    }
    cStat = config_lookup_string(&cfg, C_PATH_ROOT_INPUT,
                                (const char**)&(p_Conf->szPathRootIn));
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_PATH_ROOT_INPUT);
    }
    cStat = config_lookup_string(&cfg, C_PATH_ROOT_OUTPUT,
                                (const char**)&(p_Conf->szPathRootOut));
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_PATH_ROOT_OUTPUT);
    }
    cStat = config_lookup_string(&cfg, C_PATH_PLUGIN_SLICE,
                                (const char**)&(p_Conf->szPathPluginSlc));
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_PATH_PLUGIN_SLICE);
    }
    cStat = config_lookup_string(&cfg, C_PATH_PLUGIN_SIMILARITY,
                                (const char**)&(p_Conf->szPathPluginSim));
    if (cStat == CONFIG_FALSE) {
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_PATH_PLUGIN_SIMILARITY);
    }

    /* Load the file slicing plugin. */
    plg_Slc = (PLUGIN_SLICE*)malloc(sizeof(PLUGIN_SLICE));
    if (plg_Slc == NULL) {
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));
    }
    plg_Slc->hdle_Lib = dlopen(p_Conf->szPathPluginSlc, RTLD_LAZY);
    if (plg_Slc->hdle_Lib == NULL) {
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());
    }
    plg_Slc->Init = dlsym(plg_Slc->hdle_Lib, SYM_SLC_INIT);
    if (plg_Slc->Init == NULL) {
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());
    }
    plg_Slc->Deinit = dlsym(plg_Slc->hdle_Lib, SYM_SLC_DEINIT);
    if (plg_Slc->Deinit == NULL) {
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());
    }
    plg_Slc->GetFileSlice = dlsym(plg_Slc->hdle_Lib, SYM_SLC_GET_FILE_SLICE);
    if (plg_Slc->GetFileSlice == NULL) {
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());
    }
    cStat = plg_Slc->Init();
    if (cStat != SLC_SUCCESS) {
        EXITQ(CLS_FAIL_PLUGIN_INTERACT, EXIT);
    }

    /* Load the similarity computation plugin. */
    plg_Sim = (PLUGIN_SIMILARITY*)malloc(sizeof(PLUGIN_SIMILARITY));
    if (plg_Sim == NULL) {
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", strerror(errno));
    }
    plg_Sim->hdle_Lib = dlopen(p_Conf->szPathPluginSim, RTLD_LAZY);
    if (plg_Sim->hdle_Lib == NULL) {
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());
    }
    plg_Sim->Init = dlsym(plg_Sim->hdle_Lib, SYM_SIM_INIT);
    if (plg_Sim->Init == NULL) {
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());
    }
    plg_Sim->Deinit = dlsym(plg_Sim->hdle_Lib, SYM_SIM_DEINIT);
    if (plg_Sim->Deinit == NULL) {
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());
    }
    plg_Sim->GetHash = dlsym(plg_Sim->hdle_Lib, SYM_SIM_GET_HASH);
    if (plg_Sim->GetHash == NULL) {
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());
    }
    plg_Sim->CompareHashPair = dlsym(plg_Sim->hdle_Lib, SYM_SIM_COMPARE_HASH_PAIR);
    if (plg_Sim->CompareHashPair == NULL) {
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());
    }
    cStat = plg_Sim->Init();
    if (cStat != SIM_SUCCESS) {
        EXITQ(CLS_FAIL_PLUGIN_INTERACT, EXIT);
    }

    /* Allocate the integrated structure to record clustering progress. */
    p_Pot = (MELT_POT*)malloc(sizeof(MELT_POT));
    if (p_Pot == NULL) {
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));
    }

EXIT:
    return cRtnCode;    
}


int8_t
ClsDeinit()
{
    config_destroy(&cfg);
    if (p_Conf != NULL) {
        free(p_Conf);
    }
    if (p_Pot != NULL) {
        free(p_Pot);
    }
    if (plg_Slc != NULL) {
        if (plg_Slc->hdle_Lib != NULL) {
            plg_Slc->Deinit();
            dlclose(plg_Slc->hdle_Lib);
        }
        free(plg_Slc);
    }
    if (plg_Sim != NULL) {
        if (plg_Sim->hdle_Lib != NULL) {
            plg_Sim->Deinit();
            dlclose(plg_Sim->hdle_Lib);
        }
        free(plg_Sim);
    }

    return CLS_SUCCESS;
}


int8_t
ClsRunTask()
{
    int8_t cRtnCode = CLS_SUCCESS;
    
    cRtnCode = CrlPrepareSlice(p_Pot, p_Conf, plg_Slc);
    
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

    /* Initialize the clustering engine with user specified configurations. */
    int8_t cStat = ClsInit(szPathCfg);
    if (cStat != CLS_SUCCESS) {
        goto DEINIT;
    }

    /* Launch binary slice correlation and pattern generation. */
    cStat = ClsRunTask();

    /* Release the resources allocated by engine. */
DEINIT:
    ClsDeinit();    

EXIT:
    return cRtnCode;
}
