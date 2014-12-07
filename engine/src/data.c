
#include "data.h"


void
DsDeleteHashKey(gpointer gp_Key)
{
    return;
}


void
DsDeleteString(gpointer gp_Str)
{
    if (gp_Str)
        free(gp_Str);

    return;
}


void
DsDeleteBind(gpointer gp_Bind)
{
    if (gp_Bind)
        free(gp_Bind);

    return;
}


void
DsDeleteBlkCand(gpointer gp_BlkCand)
{
    if (gp_BlkCand) {
        BLOCK_CAND *p_BlkCand = (BLOCK_CAND*)gp_BlkCand;    
        if (p_BlkCand->a_ContAddr)
            g_array_free(p_BlkCand->a_ContAddr, true);
        free(p_BlkCand);
    }

    return;
}


void
DsDeleteGroup(gpointer gp_Val)
{
    if (gp_Val) {
        GROUP *p_Grp = (GROUP*)gp_Val;
        if (p_Grp->a_Mbr)
            g_array_free(p_Grp->a_Mbr, true);
        if (p_Grp->a_BlkCand)
            g_ptr_array_free(p_Grp->a_BlkCand, true);
        free(p_Grp);
    }

    return;
}


void
DsDeleteMeltPot(gpointer gp_Pot)
{
    if (gp_Pot) {
        MELT_POT *p_Pot = (MELT_POT*)gp_Pot;
        if (p_Pot->a_Name)
            g_ptr_array_free(p_Pot->a_Name, true);
        if (p_Pot->a_Hash)
            g_ptr_array_free(p_Pot->a_Hash, true);
        if (p_Pot->a_Slc)        
            g_ptr_array_free(p_Pot->a_Slc, true);
        if (p_Pot->h_Grp)
            g_hash_table_destroy(p_Pot->h_Grp);
        free(p_Pot);    
    }

    return;
}


int8_t
DsNewGroup(GROUP **pp_Grp)
{
    int8_t cRtnCode = CLS_SUCCESS;

    *pp_Grp = (GROUP*)malloc(sizeof(GROUP));
    if (!(*pp_Grp))
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    GROUP *p_Grp = *pp_Grp;
    p_Grp->a_Mbr = g_array_new(false, false, sizeof(uint64_t));
    if (!p_Grp->a_Mbr)
        EXIT1(CLS_FAIL_MEM_ALLOC, FREEGRP, "Error: %s.", strerror(errno));

    p_Grp->a_BlkCand = g_ptr_array_new_with_free_func(DsDeleteBlkCand);
    if (!p_Grp->a_BlkCand)
        EXIT1(CLS_FAIL_MEM_ALLOC, FREEMBR, "Error: %s.", strerror(errno));    
    goto EXIT;

FREEMBR:
    if (p_Grp->a_Mbr)
        g_array_free(p_Grp->a_Mbr, true);
FREEGRP:
    if (*pp_Grp)
        free(*pp_Grp);
EXIT:
    return cRtnCode;
}


int8_t
DsNewMeltPot(MELT_POT **pp_Pot, PLUGIN_SLICE *plg_Slc)
{
    int8_t cRtnCode = CLS_SUCCESS;

    *pp_Pot = (MELT_POT*)malloc(sizeof(MELT_POT));
    if (!(*pp_Pot))
        EXIT1(CLS_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));    

    MELT_POT *p_Pot = *pp_Pot;
    p_Pot->a_Name = g_ptr_array_new_with_free_func(DsDeleteString);    
    if (!p_Pot->a_Name)
        EXIT1(CLS_FAIL_MEM_ALLOC, FREEPOT, "Error: %s.", strerror(errno));

    p_Pot->a_Hash = g_ptr_array_new_with_free_func(DsDeleteString);
    if (!p_Pot->a_Hash)
        EXIT1(CLS_FAIL_MEM_ALLOC, FREENAME, "Error: %s.", strerror(errno));

    p_Pot->a_Slc = g_ptr_array_new_with_free_func(plg_Slc->FreeSliceArray);
    if (!p_Pot->a_Slc)
        EXIT1(CLS_FAIL_MEM_ALLOC, FREEHASH, "Error: %s.", strerror(errno));

    p_Pot->h_Grp = g_hash_table_new_full(g_int64_hash, g_int64_equal,
                                         DsDeleteHashKey, DsDeleteGroup);
    if (!p_Pot->h_Grp)
        EXIT1(CLS_FAIL_MEM_ALLOC, FREESLC, "Error: %s.", strerror(errno));
    goto EXIT;

FREESLC:
    if (p_Pot->a_Slc)
        g_ptr_array_free(p_Pot->a_Slc, true);
FREEHASH:
    if (p_Pot->a_Hash)
        g_ptr_array_free(p_Pot->a_Hash, true);
FREENAME:
    if (p_Pot->a_Name)
        g_ptr_array_free(p_Pot->a_Name, true);
FREEPOT:
    if (*pp_Pot)
        free(*pp_Pot);
EXIT:
    return cRtnCode;    
}
