/**
 *   The MIT License (MIT)
 *   Copyright (C) 2014-2017 ZongXian Shen <andy.zsshen@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <getopt.h>
#include <libconfig.h>
#include <dlfcn.h>
#include "except.h"
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
 * @param p_plgSlc      The pointer to the plugin handle.
 * @param szPath        The pathname of the plugin.
 * 
 * @return status code
 */
int8_t
_ClsInitPluginSlice(PLUGIN_SLICE **p_plgSlc, char *szPath);


/**
 * The initialization function of the similarity computation plugin.
 * 
 * @param p_plgSim      The pointer to the plugin handle.
 * @param szPath        The pathname of the plugin.
 * 
 * @return status code
 */
int8_t
_ClsInitPluginSimilarity(PLUGIN_SIMILARITY **p_plgSim, char *szPath);


/**
 * The initialization function of the pattern formatter plugin.
 * 
 * @param p_plgFmt      The pointer to the plugin handle.
 * @param szPath        The pathname of the plugin.
 * 
 * @return status code
 */
int8_t
_ClsInitPluginFormat(PLUGIN_FORMAT **p_plgFmt, char *szPath);


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
 * @param plgSlc       The plugin handle.
 */
void
_ClsDeinitPluginSlice(PLUGIN_SLICE *plgSlc);


/**
 * The deinitialization function of the similarity computation plugin.
 * 
 * @param plgSim       The plugin handle.
 */
void
_ClsDeinitPluginSimilarity(PLUGIN_SIMILARITY *plgSim);


/**
 * The deinitialization function of the pattern formatter plugin.
 * 
 * @param plgFmt       The plugin handle. 
 */
void
_ClsDeinitPluginFormat(PLUGIN_FORMAT *plgFmt);


/**
 * This function ensures that the input and output folder is ready. Note
 * the clustering process won't be started if the testing fails.
 * 
 * @param p_Conf        The pointer to the CONFIG structure.
 * 
 * @return status code
 */
int8_t
_ClsCheckFolderExistence(CONFIG *p_Conf);


/*======================================================================*
 *                Implementation for Exported Functions                 *
 *======================================================================*/
int8_t
ClsInit(char *szPathCfg)
{
    int8_t cRtnCode = SUCCESS;
    
    p_Ctx = (CONTEXT*)malloc(sizeof(CONTEXT));
    if (!p_Ctx)
        EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    p_Ctx->p_Conf = NULL;
    p_Ctx->p_Pot = NULL;
    p_Ctx->plgSlc = NULL;
    p_Ctx->plgSim = NULL;
    p_Ctx->plgFmt = NULL;

    /* Resolve the user specified configuration. */
    int8_t cStat = _ClsInitConfig(&(p_Ctx->p_Conf), szPathCfg);
    if (cStat != SUCCESS)
        EXITQ(cStat, EXIT);

    /* Check the existence of the input and output folder. */
    cStat = _ClsCheckFolderExistence(p_Ctx->p_Conf);
    if (cStat != SUCCESS)
        EXITQ(cStat, EXIT);

    /* Load the relevant plugins. */
    CONFIG *p_Conf = p_Ctx->p_Conf;
    cStat = _ClsInitPluginSlice(&(p_Ctx->plgSlc), p_Conf->szPathPluginSlc);
    if (cStat != SUCCESS)
        EXITQ(cStat, EXIT);

    cStat = _ClsInitPluginSimilarity(&(p_Ctx->plgSim), p_Conf->szPathPluginSim);
    if (cStat != SUCCESS)
        EXITQ(cStat, EXIT);

    cStat = _ClsInitPluginFormat(&(p_Ctx->plgFmt), p_Conf->szPathPluginFmt);
    if (cStat != SUCCESS)
        EXITQ(cStat, EXIT);

    /* Allocate the MELT_POT structure to record the clustering progress. */
    cStat = DsNewMeltPot(&(p_Ctx->p_Pot));

EXIT:
    return cRtnCode;    
}


int8_t
ClsDeinit()
{
    if (!p_Ctx)
        return SUCCESS;

    DsDeleteMeltPot(p_Ctx->p_Pot);
    _ClsDeinitConfig(p_Ctx->p_Conf);
    _ClsDeinitPluginSlice(p_Ctx->plgSlc);
    _ClsDeinitPluginSimilarity(p_Ctx->plgSim);
    _ClsDeinitPluginFormat(p_Ctx->plgFmt);

    free(p_Ctx);
    return SUCCESS;
}


int8_t
ClsRunTask()
{
    int8_t cRtnCode = SUCCESS;

    CrlSetContext(p_Ctx);
    int8_t cStat = CrlPrepareSlice();
    if (cStat != SUCCESS)
        EXIT1(cStat, EXIT, "%s\n", SLC_GEN_FAIL);
    SPEW1("%s\n", SLC_GEN_SUCC);

    cStat = CrlCorrelateSlice();
    if (cStat != SUCCESS)
        EXIT1(cStat, EXIT, "%s\n", SLC_CRL_FAIL);
    SPEW1("%s\n", SLC_CRL_SUCC);

    PtnSetContext(p_Ctx);
    cStat = PtnCraftPattern();
    if (cStat != SUCCESS)
        EXIT1(cStat, EXIT, "%s\n", PTN_GEN_FAIL);
    SPEW1("%s\n", PTN_GEN_SUCC);    

    cStat = PtnOutputResult();
    if (cStat != SUCCESS)
        EXIT1(cStat, EXIT, "%s\n", PTN_OUT_FAIL);
    SPEW1("%s\n", PTN_OUT_SUCC);

EXIT:
    return cRtnCode;
}


int8_t
main(int argc, char **argv, char **envp)
{
    int8_t cRtnCode = SUCCESS;

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
        EXIT1(FAIL_OPT_PARSE, EXIT, "Error: %s.", FAIL_OPT_PARSE_CONF);

    int8_t cStat = ClsInit(szPathCfg);
    if (cStat != SUCCESS)
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
    int8_t cRtnCode = SUCCESS;

    config_init(&cfg);
    int8_t cStat = config_read_file(&cfg, szPath);
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_FILE_IO, EXIT, "Error: %s.", config_error_text(&cfg));

    *pp_Conf = (CONFIG*)malloc(sizeof(CONFIG));
    if (!(*pp_Conf))
        EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    CONFIG *p_Conf = *pp_Conf;
    int iVal;
    cStat = config_lookup_bool(&cfg, C_FLAG_COMMENT, &iVal);
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_FLAG_COMMENT);
    p_Conf->bComt = (bool)iVal;
    
    cStat = config_lookup_int(&cfg, C_COUNT_THREAD, &iVal);    
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_COUNT_THREAD);
    p_Conf->ucCntThrd = (uint8_t)iVal;

    cStat = config_lookup_int(&cfg, C_THRESHOLD_SIMILARITY, &iVal);    
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_THRESHOLD_SIMILARITY);        
    p_Conf->ucScoreSim = (uint8_t)iVal;

    cStat = config_lookup_int(&cfg, C_COUNT_HEX_BLOCK, &iVal);
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_COUNT_HEX_BLOCK);
    p_Conf->ucCntBlk = (uint8_t)iVal;

    cStat = config_lookup_int(&cfg, C_SIZE_HEX_BLOCK, &iVal);
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_SIZE_HEX_BLOCK);
    p_Conf->ucSizeBlk = (uint8_t)iVal;

    cStat = config_lookup_int(&cfg, C_SIZE_SLICE, &iVal);
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_SIZE_SLICE);
    p_Conf->usSizeSlc = (uint16_t)iVal;

    cStat = config_lookup_int(&cfg, C_RATIO_NOISE, &iVal);
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_RATIO_NOISE);
    p_Conf->ucRatNoise = (uint8_t)iVal;

    cStat = config_lookup_int(&cfg, C_RATIO_WILDCARD, &iVal);
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_RATIO_WILDCARD);
    p_Conf->ucRatWild = (uint8_t)iVal;

    cStat = config_lookup_int(&cfg, C_IO_BANDWIDTH, &iVal);
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_IO_BANDWIDTH);
    p_Conf->ucIoBand = (uint8_t)iVal;

    cStat = config_lookup_int(&cfg, C_SIZE_TRUNCATE_GROUP, &iVal);
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_SIZE_TRUNCATE_GROUP);
    p_Conf->ucSizeTruncGrp = (uint8_t)iVal;

    cStat = config_lookup_string(&cfg, C_PATH_ROOT_INPUT,
                                (const char**)&(p_Conf->szPathRootIn));
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_PATH_ROOT_INPUT);

    cStat = config_lookup_string(&cfg, C_PATH_ROOT_OUTPUT,
                                (const char**)&(p_Conf->szPathRootOut));
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_PATH_ROOT_OUTPUT);

    cStat = config_lookup_string(&cfg, C_PATH_PLUGIN_SLICE,
                                (const char**)&(p_Conf->szPathPluginSlc));
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_PATH_PLUGIN_SLICE);

    cStat = config_lookup_string(&cfg, C_PATH_PLUGIN_SIMILARITY,
                                (const char**)&(p_Conf->szPathPluginSim));
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_PATH_PLUGIN_SIMILARITY);

    cStat = config_lookup_string(&cfg, C_PATH_PLUGIN_FORMAT,
                                (const char**)&(p_Conf->szPathPluginFmt));
    if (cStat == CONFIG_FALSE)
        EXIT1(FAIL_CONF_PARSE, EXIT, "Error: %s missed.", C_PATH_PLUGIN_FORMAT);                            

EXIT:
    return cRtnCode;
}


int8_t
_ClsInitPluginSlice(PLUGIN_SLICE **p_plgSlc, char *szPath)
{
    int8_t cRtnCode = SUCCESS;

    *p_plgSlc = (PLUGIN_SLICE*)malloc(sizeof(PLUGIN_SLICE));
    if (!(*p_plgSlc))
        EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    PLUGIN_SLICE *plgSlc = *p_plgSlc;
    plgSlc->hdle_Lib = dlopen(szPath, RTLD_LAZY);
    if (!plgSlc->hdle_Lib)
        EXIT1(FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plgSlc->Init = dlsym(plgSlc->hdle_Lib, SYM_SLC_INIT);
    if (!plgSlc->Init)
        EXIT1(FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plgSlc->Deinit = dlsym(plgSlc->hdle_Lib, SYM_SLC_DEINIT);
    if (!plgSlc->Deinit)
        EXIT1(FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plgSlc->GetFileSlice = dlsym(plgSlc->hdle_Lib, SYM_SLC_GET_FILE_SLICE);
    if (!plgSlc->GetFileSlice)
        EXIT1(FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    int8_t cStat = plgSlc->Init();
    if (cStat != SUCCESS)
        EXITQ(cStat, EXIT);

EXIT:
    return cRtnCode;
}


int8_t
_ClsInitPluginSimilarity(PLUGIN_SIMILARITY **p_plgSim, char *szPath)
{
    int8_t cRtnCode = SUCCESS;

    *p_plgSim = (PLUGIN_SIMILARITY*)malloc(sizeof(PLUGIN_SIMILARITY));
    if (!(*p_plgSim))
        EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    PLUGIN_SIMILARITY *plgSim = *p_plgSim;
    plgSim->hdle_Lib = dlopen(szPath, RTLD_LAZY);
    if (!plgSim->hdle_Lib)
        EXIT1(FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plgSim->Init = dlsym(plgSim->hdle_Lib, SYM_SIM_INIT);
    if (!plgSim->Init)
        EXIT1(FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plgSim->Deinit = dlsym(plgSim->hdle_Lib, SYM_SIM_DEINIT);
    if (!plgSim->Deinit)
        EXIT1(FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plgSim->GetHash = dlsym(plgSim->hdle_Lib, SYM_SIM_GET_HASH);
    if (!plgSim->GetHash)
        EXIT1(FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plgSim->CompareHashPair = dlsym(plgSim->hdle_Lib, SYM_SIM_COMPARE_HASH_PAIR);
    if (!plgSim->CompareHashPair)
        EXIT1(FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    int8_t cStat = plgSim->Init();
    if (cStat != SUCCESS)
        EXITQ(cStat, EXIT);

EXIT:
    return cRtnCode;
}


int8_t
_ClsInitPluginFormat(PLUGIN_FORMAT **p_plgFmt, char *szPath)
{
    int8_t cRtnCode = SUCCESS;

    *p_plgFmt = (PLUGIN_FORMAT*)malloc(sizeof(PLUGIN_FORMAT));
    if (!(*p_plgFmt))
        EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    PLUGIN_FORMAT *plgFmt = *p_plgFmt;
    plgFmt->hdle_Lib = dlopen(szPath, RTLD_LAZY);
    if (!plgFmt->hdle_Lib)
        EXIT1(FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plgFmt->Init = dlsym(plgFmt->hdle_Lib, SYM_FMT_INIT);
    if (!plgFmt->Init)
        EXIT1(FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plgFmt->Deinit = dlsym(plgFmt->hdle_Lib, SYM_FMT_DEINIT);
    if (!plgFmt->Deinit)
        EXIT1(FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    plgFmt->Print = dlsym(plgFmt->hdle_Lib, SYM_FMT_PRINT);
    if (!plgFmt->Print)
        EXIT1(FAIL_PLUGIN_RESOLVE, EXIT, "Error: %s.", dlerror());

    int8_t cStat = plgFmt->Init();
    if (cStat != SUCCESS)
        EXITQ(cStat, EXIT);

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
_ClsDeinitPluginSlice(PLUGIN_SLICE *plgSlc)
{
    if (plgSlc) {
        if (plgSlc->hdle_Lib) {
            plgSlc->Deinit();
            dlclose(plgSlc->hdle_Lib);
        }
        free(plgSlc);
    }

    return;
}


void
_ClsDeinitPluginSimilarity(PLUGIN_SIMILARITY *plgSim)
{
    if (plgSim) {
        if (plgSim->hdle_Lib) {
            plgSim->Deinit();
            dlclose(plgSim->hdle_Lib);
        }
        free(plgSim);
    }

    return;
}


void
_ClsDeinitPluginFormat(PLUGIN_FORMAT *plgFmt)
{
    if (plgFmt) {
        if (plgFmt->hdle_Lib) {
            plgFmt->Deinit();
            dlclose(plgFmt->hdle_Lib);
        }
        free(plgFmt);
    }

    return;
}


int8_t
_ClsCheckFolderExistence(CONFIG *p_Conf)
{
    int8_t cRtnCode = SUCCESS;

    /* Check if the input folder exists. */
    struct stat stDir;
    int8_t cStat = stat(p_Conf->szPathRootIn, &stDir);
    if (cStat != 0)
        EXIT1(FAIL_FILE_IO, EXIT, "Error: %s.", strerror(errno));

    /* Create the output folder if it does not exist. */
    cStat = stat(p_Conf->szPathRootOut, &stDir);
    if (cStat != 0) {
        cStat = mkdir(p_Conf->szPathRootOut, 0700);
        if (cStat != 0)
            EXIT1(FAIL_FILE_IO, EXIT, "Error: %s.", strerror(errno));
    }

EXIT:
    return cRtnCode;
}
