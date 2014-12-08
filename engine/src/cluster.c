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


static config_t cfg;
static CONTEXT *p_Ctx;


/*======================================================================*
 *                 Declaration for Internal Functions                   *
 *======================================================================*/
/**
 * The initialization function of CONFIG structure.
 * 
 * @param pp_Conf       The pointer to the pointer of target structure.
 * @param szPath        The pathname of the configuration file.
 * 
 * @return status code 
 */
int8_t
_ClsInitConfig(CONFIG **pp_Conf, char *szPath);


/**
 * The initialization function of the file slicing plugin.
 * 
 * @param p_plg_Slc     The pointer to the plugin handle.
 * @param szPath        The pathname of the plugin.
 * 
 * @return status code
 */
int8_t
_ClsInitPluginSlice(PLUGIN_SLICE **p_plg_Slc, char *szPath);


/**
 * The initialization function of the similarity computation plugin.
 * 
 * @param p_plg_Sim     The pointer to the plugin handle.
 * @param szPath        The pathname of the plugin.
 * 
 * @return status code
 */
int8_t
_ClsInitPluginSimilarity(PLUGIN_SIMILARITY **p_plg_Sim, char *szPath);


/**
 * The deinitialization function of CONFIG structure.
 * 
 * @param p_Conf        The pointer to the target structure.
 */
void
_ClsDeinitConfig(CONFIG *p_Conf);


/**
 * The deinitialization function of the file slicing plugin.
 * 
 * @param plg_Slc       The plugin handle.
 */
void
_ClsDeinitPluginSlice(PLUGIN_SLICE *plg_Slc);


/**
 * The deinitialization function of the similarity computation plugin.
 * 
 * @param plg_Sim       The plugin handle.
 */
void
_ClsDeinitPluginSimilarity(PLUGIN_SIMILARITY *plg_Sim);


/*======================================================================*
 *                Implementation for Exported Functions                 *
 *======================================================================*/
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

    /* Resolve the user specified configuration. */
    int8_t cStat = _ClsInitConfig(&(p_Ctx->p_Conf), szPathCfg);
    if (cStat != CLS_SUCCESS)
        EXITQ(cStat, EXIT);

    /* Load the file slicing and similarity computation plugins. */
    CONFIG *p_Conf = p_Ctx->p_Conf;
    cStat = _ClsInitPluginSlice(&(p_Ctx->plg_Slc), p_Conf->szPathPluginSlc);
    if (cStat != CLS_SUCCESS)
        EXITQ(cStat, EXIT);

    cStat = _ClsInitPluginSimilarity(&(p_Ctx->plg_Sim), p_Conf->szPathPluginSim);
    if (cStat != CLS_SUCCESS)
        EXITQ(cStat, EXIT);

    /* Allocate the MELT_POT structure to record the clustering progress. */
    cStat = DsNewMeltPot(&(p_Ctx->p_Pot), p_Ctx->plg_Slc);

EXIT:
    return cRtnCode;    
}


int8_t
ClsDeinit()
{
    if (!p_Ctx)
        return CLS_SUCCESS;

    DsDeleteMeltPot(p_Ctx->p_Pot);
    _ClsDeinitConfig(p_Ctx->p_Conf);
    _ClsDeinitPluginSlice(p_Ctx->plg_Slc);
    _ClsDeinitPluginSimilarity(p_Ctx->plg_Sim);

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

    int8_t cStat = ClsInit(szPathCfg);
    if (cStat != CLS_SUCCESS)
        goto DEINIT;

    cStat = ClsRunTask();

DEINIT:
    ClsDeinit();    
EXIT:
    return cRtnCode;
}


/*======================================================================*
 *                Implementation for Internal Functions                 *
 *======================================================================*/
 int8_t
_ClsInitConfig(CONFIG **pp_Conf, char *szPath)
{
    int8_t cRtnCode = CLS_SUCCESS;

    int8_t cStat = config_read_file(&cfg, szPath);
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_FILE_IO, EXIT, "Error: %s.", config_error_text(&cfg));

    *pp_Conf = (CONFIG*)malloc(sizeof(CONFIG));
    if (!(*pp_Conf))
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    CONFIG *p_Conf = *pp_Conf;
    cStat = config_lookup_int(&cfg, C_COUNT_THREAD, &(p_Conf->ucCntThrd));    
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_COUNT_THREAD);        

    cStat = config_lookup_int(&cfg, C_THRESHOLD_SIMILARITY, &(p_Conf->ucScoreSim));    
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_THRESHOLD_SIMILARITY);        

    cStat = config_lookup_int(&cfg, C_COUNT_HEX_BLOCK, &(p_Conf->ucCntBlk));
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_COUNT_HEX_BLOCK);

    cStat = config_lookup_int(&cfg, C_SIZE_HEX_BLOCK, &(p_Conf->ucSizeBlk));
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_SIZE_HEX_BLOCK);

    cStat = config_lookup_int(&cfg, C_SIZE_SLICE, &(p_Conf->usSizeSlc));
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_SIZE_SLICE);

    cStat = config_lookup_int(&cfg, C_RATIO_NOISE, &(p_Conf->ucRatNoise));
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_RATIO_NOISE);

    cStat = config_lookup_int(&cfg, C_RATIO_WILDCARD, &(p_Conf->ucRatWild));
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_RATIO_WILDCARD);

    cStat = config_lookup_int(&cfg, C_IO_BANDWIDTH, &(p_Conf->ucIoBand));
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_IO_BANDWIDTH);

    cStat = config_lookup_bool(&cfg, C_TRUNCATE_TRIVIAL_GROUP, &(p_Conf->bTrunc));
    if (cStat == CONFIG_FALSE)
        EXIT1(CLS_FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_TRUNCATE_TRIVIAL_GROUP);
    
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
    
EXIT:
    return cRtnCode;
}


int8_t
_ClsInitPluginSlice(PLUGIN_SLICE **p_plg_Slc, char *szPath)
{
    int8_t cRtnCode = CLS_SUCCESS;

    *p_plg_Slc = (PLUGIN_SLICE*)malloc(sizeof(PLUGIN_SLICE));
    if (!(*p_plg_Slc))
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    PLUGIN_SLICE *plg_Slc = *p_plg_Slc;
    plg_Slc->hdle_Lib = dlopen(szPath, RTLD_LAZY);
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

    plg_Slc->DeleteSlice = dlsym(plg_Slc->hdle_Lib, SYM_SLC_FREE_SLICE_ARRAY);
    if (!plg_Slc->DeleteSlice)
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    int8_t cStat = plg_Slc->Init();
    if (cStat != SLC_SUCCESS)
        EXITQ(CLS_FAIL_PLUGIN_INTERACT, EXIT);

EXIT:
    return cRtnCode;
}


int8_t
_ClsInitPluginSimilarity(PLUGIN_SIMILARITY **p_plg_Sim, char *szPath)
{
    int8_t cRtnCode = CLS_SUCCESS;
    
    *p_plg_Sim = (PLUGIN_SIMILARITY*)malloc(sizeof(PLUGIN_SIMILARITY));
    if (!(*p_plg_Sim))
        EXIT1(CLS_FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", strerror(errno));

    PLUGIN_SIMILARITY *plg_Sim = *p_plg_Sim;
    plg_Sim->hdle_Lib = dlopen(szPath, RTLD_LAZY);
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

    int8_t cStat = plg_Sim->Init();
    if (cStat != SIM_SUCCESS)
        EXITQ(CLS_FAIL_PLUGIN_INTERACT, EXIT);

EXIT:
    return cRtnCode;
}


void
_ClsDeinitConfig(CONFIG *p_Conf)
{
    config_destroy(&cfg);
    if (p_Conf)
        free(p_Conf);

    return;
}


void
_ClsDeinitPluginSlice(PLUGIN_SLICE *plg_Slc)
{
    if (plg_Slc) {
        if (plg_Slc->hdle_Lib) {
            plg_Slc->Deinit();
            dlclose(plg_Slc->hdle_Lib);
        }
        free(plg_Slc);
    }

    return;
}


void
_ClsDeinitPluginSimilarity(PLUGIN_SIMILARITY *plg_Sim)
{
    if (plg_Sim) {
        if (plg_Sim->hdle_Lib) {
            plg_Sim->Deinit();
            dlclose(plg_Sim->hdle_Lib);
        }
        free(plg_Sim);
    }

    return;
}
