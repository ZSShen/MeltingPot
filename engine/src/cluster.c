#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
CONTEXT *p_Ctx;


int8_t
ClsInit(char *szPathCfg)
{
    int8_t cRtnCode = CLS_SUCCESS;
    
    config_init(&cfg);
    p_Ctx = (CONTEXT*)malloc(sizeof(CONTEXT));
    if (!p_Ctx)
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    p_Ctx->p_Conf = NULL;
    p_Ctx->p_Pot = NULL;
    p_Ctx->plg_Slc = NULL;
    p_Ctx->plg_Sim = NULL;

    int8_t cStat = config_read_file(&cfg, szPathCfg);
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_FILE_IO, EXIT, "Error: %s.", config_error_text(&cfg));

    p_Ctx->p_Conf = (CONFIG*)malloc(sizeof(CONFIG));
    if (!p_Ctx->p_Conf)
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    /* Resolve the user specified configuration. */
    CONFIG *p_Conf = p_Ctx->p_Conf;
    cStat = config_lookup_int(&cfg, C_COUNT_THREAD, (int64_t*)&(p_Conf->ucCntThrd));    
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_COUNT_THREAD);        

    cStat = config_lookup_int(&cfg, C_THRESHOLD_SIMILARITY, (int64_t*)&(p_Conf->ucScoreSim));    
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_THRESHOLD_SIMILARITY);        

    cStat = config_lookup_int(&cfg, C_COUNT_HEX_BLOCK, (int64_t*)&(p_Conf->ucCntBlk));
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_COUNT_HEX_BLOCK);

    cStat = config_lookup_int(&cfg, C_SIZE_HEX_BLOCK, (int64_t*)&(p_Conf->ucSizeBlk));
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_SIZE_HEX_BLOCK);

    cStat = config_lookup_int(&cfg, C_SIZE_SLICE, (int64_t*)&(p_Conf->usSizeSlc));
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_SIZE_SLICE);

    cStat = config_lookup_string(&cfg, C_PATH_ROOT_INPUT,
                                (const char**)&(p_Conf->szPathRootIn));
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_PATH_ROOT_INPUT);

    cStat = config_lookup_string(&cfg, C_PATH_ROOT_OUTPUT,
                                (const char**)&(p_Conf->szPathRootOut));
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_PATH_ROOT_OUTPUT);

    cStat = config_lookup_string(&cfg, C_PATH_PLUGIN_SLICE,
                                (const char**)&(p_Conf->szPathPluginSlc));
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_PATH_PLUGIN_SLICE);

    cStat = config_lookup_string(&cfg, C_PATH_PLUGIN_SIMILARITY,
                                (const char**)&(p_Conf->szPathPluginSim));
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_PATH_PLUGIN_SIMILARITY);

    /* Load the file slicing plugin. */
    p_Ctx->plg_Slc = (PLUGIN_SLICE*)malloc(sizeof(PLUGIN_SLICE));
    if (!p_Ctx->plg_Slc)
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    PLUGIN_SLICE *plg_Slc = p_Ctx->plg_Slc;
    plg_Slc->hdle_Lib = dlopen(p_Conf->szPathPluginSlc, RTLD_LAZY);
    if (!plg_Slc->hdle_Lib)
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plg_Slc->Init = dlsym(plg_Slc->hdle_Lib, SYM_SLC_INIT);
    if (!plg_Slc->Init)
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plg_Slc->Deinit = dlsym(plg_Slc->hdle_Lib, SYM_SLC_DEINIT);
    if (!plg_Slc->Deinit)
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plg_Slc->GetFileSlice = dlsym(plg_Slc->hdle_Lib, SYM_SLC_GET_FILE_SLICE);
    if (!plg_Slc->GetFileSlice)
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plg_Slc->FreeSliceArray = dlsym(plg_Slc->hdle_Lib, SYM_SLC_FREE_SLICE_ARRAY);
    if (!plg_Slc->FreeSliceArray)
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    cStat = plg_Slc->Init();
    if (cStat != SLC_SUCCESS)
        EXITQ(CLS_FAIL_PLUGIN_INTERACT, EXIT);

    /* Load the similarity computation plugin. */
    p_Ctx->plg_Sim = (PLUGIN_SIMILARITY*)malloc(sizeof(PLUGIN_SIMILARITY));
    if (!p_Ctx->plg_Sim)
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", strerror(errno));

    PLUGIN_SIMILARITY *plg_Sim = p_Ctx->plg_Sim;
    plg_Sim->hdle_Lib = dlopen(p_Conf->szPathPluginSim, RTLD_LAZY);
    if (!plg_Sim->hdle_Lib)
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plg_Sim->Init = dlsym(plg_Sim->hdle_Lib, SYM_SIM_INIT);
    if (!plg_Sim->Init)
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plg_Sim->Deinit = dlsym(plg_Sim->hdle_Lib, SYM_SIM_DEINIT);
    if (!plg_Sim->Deinit)
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plg_Sim->GetHash = dlsym(plg_Sim->hdle_Lib, SYM_SIM_GET_HASH);
    if (!plg_Sim->GetHash)
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plg_Sim->CompareHashPair = dlsym(plg_Sim->hdle_Lib, SYM_SIM_COMPARE_HASH_PAIR);
    if (!plg_Sim->CompareHashPair)
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    cStat = plg_Sim->Init();
    if (cStat != SIM_SUCCESS)
        EXITQ(CLS_FAIL_PLUGIN_INTERACT, EXIT);

    /* Allocate the integrated structure to record clustering progress. */
    p_Ctx->p_Pot = (MELT_POT*)malloc(sizeof(MELT_POT));
    if (!p_Ctx->p_Pot)
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    p_Ctx->p_Pot->a_Name = NULL;
    p_Ctx->p_Pot->a_Hash = NULL;
    p_Ctx->p_Pot->a_Slc = NULL;
    p_Ctx->p_Pot->a_Grp = NULL;

EXIT:
    return cRtnCode;    
}


int8_t
ClsDeinit()
{
    config_destroy(&cfg);
    
    if (!p_Ctx)
        return CLS_SUCCESS;
    
    if (p_Ctx->p_Conf)
        free(p_Ctx->p_Conf);

    if (p_Ctx->p_Pot) {
        if (p_Ctx->p_Pot->a_Name)
            g_ptr_array_free(p_Ctx->p_Pot->a_Name, true);

        if (p_Ctx->p_Pot->a_Hash)
            g_ptr_array_free(p_Ctx->p_Pot->a_Hash, true);

        if (p_Ctx->p_Pot->a_Slc)
            g_ptr_array_free(p_Ctx->p_Pot->a_Slc, true);

        free(p_Ctx->p_Pot);
    }
    if (p_Ctx->plg_Slc) {
        if (p_Ctx->plg_Slc->hdle_Lib) {
            p_Ctx->plg_Slc->Deinit();
            dlclose(p_Ctx->plg_Slc->hdle_Lib);
        }
        
        free(p_Ctx->plg_Slc);
    }
    if (p_Ctx->plg_Sim) {
        if (p_Ctx->plg_Sim->hdle_Lib) {
            p_Ctx->plg_Sim->Deinit();
            dlclose(p_Ctx->plg_Sim->hdle_Lib);
        }
        
        free(p_Ctx->plg_Sim);
    }

    free(p_Ctx);
    return CLS_SUCCESS;
}


int8_t
ClsRunTask()
{
    int8_t cRtnCode = CLS_SUCCESS;

    CrlSetContext(p_Ctx);
    int8_t cStat = CrlPrepareSlice();
    if (cStat != CLS_SUCCESS)
        EXIT1(cStat, EXIT, "Notice: %s.", SLICE_GENERATION_FAIL);
    SPEW1("Notice: %s.", SLICE_GENERATION_SUCC);

    cStat = CrlCorrelateSlice();
    if (cStat != CLS_SUCCESS)
        EXIT1(cStat, EXIT, "Notice: %s.", SLICE_CORRELATION_FAIL);
    SPEW1("Notice: %s.", SLICE_CORRELATION_SUCC);

EXIT:
    return cRtnCode;
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
            case OPT_HELP:
                break;
        }
    }
    if (!szPathCfg)
        EXIT1(CLS_FAIL_OPT_PARSE, EXIT, "Error: %s.", FAIL_OPT_PARSE_CONF);

    /* Initialize the clustering engine with user specified configurations. */
    int8_t cStat = ClsInit(szPathCfg);
    if (cStat != CLS_SUCCESS)
        goto DEINIT;

    /* Launch binary slice correlation and pattern generation. */
    cStat = ClsRunTask();

    /* Release the resources allocated by the engine. */
DEINIT:
    ClsDeinit();    
EXIT:
    return cRtnCode;
}
