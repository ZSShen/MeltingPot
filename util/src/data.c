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


#include "except.h"
#include "spew.h"
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
DsDeleteArrayPath(gpointer gp_APath)
{
    if (gp_APath) {
        GPtrArray *a_Path = (GPtrArray*)gp_APath;
        g_ptr_array_free(a_Path, true);
    }

    return;
}


void
DsDeleteContentAddr(gpointer gp_CtnAddr)
{
    if (gp_CtnAddr)
        free(gp_CtnAddr);

    return;
}


void
DsDeleteBlkCand(gpointer gp_BlkCand)
{
    if (gp_BlkCand) {
        BLOCK_CAND *p_BlkCand = (BLOCK_CAND*)gp_BlkCand;    
        if (p_BlkCand->t_CtnAddr)
            g_tree_destroy(p_BlkCand->t_CtnAddr);    
        if (p_BlkCand->a_usCtn)
            free(p_BlkCand->a_usCtn);    
        free(p_BlkCand);
    }

    return;
}


void
DsDeleteSlice(gpointer gp_Slc)
{
    if (gp_Slc)
        free(gp_Slc);

    return;
}


void
DsDeleteGroup(gpointer gp_Grp)
{
    if (gp_Grp) {
        GROUP *p_Grp = (GROUP*)gp_Grp;
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
        if (p_Pot->a_Path)
            g_ptr_array_free(p_Pot->a_Path, true);
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


void
DsDeletePatternText(PATTERN_TEXT *p_Text)
{
    if (p_Text->gszSecStr)
        g_string_free(p_Text->gszSecStr, true);
    if (p_Text->gszSecCond)
        g_string_free(p_Text->gszSecCond, true);
    if (p_Text->gszComt)
        g_string_free(p_Text->gszComt, true);
    if (p_Text->gszFullPtn)
        g_string_free(p_Text->gszFullPtn, true);    
    if (p_Text)
        free(p_Text);

    return;
}


int8_t
DsNewBlockCand(BLOCK_CAND **pp_BlkCand, uint8_t ucSizeCtn)
{
    int8_t cRtnCode = SUCCESS;

    *pp_BlkCand = (BLOCK_CAND*)malloc(sizeof(BLOCK_CAND));
    if (!(*pp_BlkCand))
        EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    BLOCK_CAND *p_BlkCand = *pp_BlkCand;
    p_BlkCand->t_CtnAddr = g_tree_new_full(DsCompContentAddrPlus, NULL, 
                                           DsDeleteContentAddr, DsDeleteArrayPath);
    if (!p_BlkCand->t_CtnAddr)
        EXIT1(FAIL_MEM_ALLOC, FREEBLK, "Error: %s.", strerror(errno));

    p_BlkCand->a_usCtn = (uint16_t*)malloc(sizeof(uint16_t) * ucSizeCtn);
    if (!p_BlkCand->a_usCtn)
        EXIT1(FAIL_MEM_ALLOC, FREEADDR, "Error: %s.", strerror(errno));

    p_BlkCand->ucCntNoise = 0;
    p_BlkCand->ucCntWild = 0;
    p_BlkCand->ucSizeCtn = ucSizeCtn;
    goto EXIT;

FREEADDR:
    if (p_BlkCand->t_CtnAddr)
        g_tree_destroy(p_BlkCand->t_CtnAddr);
FREEBLK:
    if (*pp_BlkCand)
        free(*pp_BlkCand);
EXIT:
    return cRtnCode;
}


int8_t
DsNewGroup(GROUP **pp_Grp)
{
    int8_t cRtnCode = SUCCESS;

    *pp_Grp = (GROUP*)malloc(sizeof(GROUP));
    if (!(*pp_Grp))
        EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    GROUP *p_Grp = *pp_Grp;
    p_Grp->a_Mbr = g_array_new(false, false, sizeof(uint64_t));
    if (!p_Grp->a_Mbr)
        EXIT1(FAIL_MEM_ALLOC, FREEGRP, "Error: %s.", strerror(errno));

    p_Grp->a_BlkCand = g_ptr_array_new_with_free_func(DsDeleteBlkCand);
    if (!p_Grp->a_BlkCand)
        EXIT1(FAIL_MEM_ALLOC, FREEMBR, "Error: %s.", strerror(errno));    
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
DsNewMeltPot(MELT_POT **pp_Pot)
{
    int8_t cRtnCode = SUCCESS;

    *pp_Pot = (MELT_POT*)malloc(sizeof(MELT_POT));
    if (!(*pp_Pot))
        EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));    

    MELT_POT *p_Pot = *pp_Pot;
    p_Pot->a_Path = g_ptr_array_new_with_free_func(DsDeleteString);    
    if (!p_Pot->a_Path)
        EXIT1(FAIL_MEM_ALLOC, FREEPOT, "Error: %s.", strerror(errno));

    p_Pot->a_Hash = g_ptr_array_new_with_free_func(DsDeleteString);
    if (!p_Pot->a_Hash)
        EXIT1(FAIL_MEM_ALLOC, FREENAME, "Error: %s.", strerror(errno));

    p_Pot->a_Slc = g_ptr_array_new_with_free_func(DsDeleteSlice);
    if (!p_Pot->a_Slc)
        EXIT1(FAIL_MEM_ALLOC, FREEHASH, "Error: %s.", strerror(errno));

    p_Pot->h_Grp = g_hash_table_new_full(g_int64_hash, g_int64_equal,
                                         DsDeleteHashKey, DsDeleteGroup);
    if (!p_Pot->h_Grp)
        EXIT1(FAIL_MEM_ALLOC, FREESLC, "Error: %s.", strerror(errno));
    goto EXIT;

FREESLC:
    if (p_Pot->a_Slc)
        g_ptr_array_free(p_Pot->a_Slc, true);
FREEHASH:
    if (p_Pot->a_Hash)
        g_ptr_array_free(p_Pot->a_Hash, true);
FREENAME:
    if (p_Pot->a_Path)
        g_ptr_array_free(p_Pot->a_Path, true);
FREEPOT:
    if (*pp_Pot)
        free(*pp_Pot);
EXIT:
    return cRtnCode;    
}


int8_t
DsNewPatternText(PATTERN_TEXT **pp_Text)
{
    int8_t cRtnCode = SUCCESS;

    *pp_Text = (PATTERN_TEXT*)malloc(sizeof(PATTERN_TEXT));
    if (!(*pp_Text))
        EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    PATTERN_TEXT *p_Text = *pp_Text;
    p_Text->gszSecStr = NULL;
    p_Text->gszSecCond = NULL;
    p_Text->gszComt = NULL;
    p_Text->gszFullPtn = NULL;

    p_Text->gszSecStr = g_string_new(NULL);
    if (!p_Text->gszSecStr)
        EXIT1(FAIL_MEM_ALLOC, FREETEXT, "Error: %s.", strerror(errno));
    p_Text->gszSecCond = g_string_new(NULL);
    if (!p_Text->gszSecCond)
        EXIT1(FAIL_MEM_ALLOC, FREEGSTR, "Error: %s.", strerror(errno));
    p_Text->gszComt = g_string_new(NULL);
    if (!p_Text->gszComt)
        EXIT1(FAIL_MEM_ALLOC, FREEGSTR, "Error: %s.", strerror(errno));   
    p_Text->gszFullPtn = g_string_new(NULL);
    if (!p_Text->gszFullPtn)
        EXIT1(FAIL_MEM_ALLOC, FREEGSTR, "Error: %s.", strerror(errno));
    goto EXIT;

FREEGSTR:
    if (p_Text->gszComt)
        g_string_free(p_Text->gszComt, true);
    if (p_Text->gszSecCond)
        g_string_free(p_Text->gszSecCond, true);
    if (p_Text->gszSecStr)
        g_string_free(p_Text->gszSecStr, true);
FREETEXT:
    if (*pp_Text)
        free(*pp_Text);
EXIT:
    return cRtnCode;
}


int
DsCompBlockCandNoise(const void *vp_Src, const void *vp_Tge)
{
    BLOCK_CAND *p_Src = *(BLOCK_CAND**)vp_Src;
    BLOCK_CAND *p_Tge = *(BLOCK_CAND**)vp_Tge;
    return p_Src->ucCntNoise - p_Tge->ucCntNoise;
}


int
DsCompBlockCandWildCard(const void *vp_Src, const void *vp_Tge)
{
    BLOCK_CAND *p_Src = *(BLOCK_CAND**)vp_Src;
    BLOCK_CAND *p_Tge = *(BLOCK_CAND**)vp_Tge;
    if (p_Src->ucCntWild == p_Tge->ucCntWild)
        return (int16_t)p_Src->ucCntNoise - (int16_t)p_Tge->ucCntNoise;
    return (int16_t)p_Src->ucCntWild - (int16_t)p_Tge->ucCntWild;
}


int
DsCompContentAddr(const void *vp_Src, const void *vp_Tge)
{
    CONTENT_ADDR *p_Src = (CONTENT_ADDR*)vp_Src;
    CONTENT_ADDR *p_Tge = (CONTENT_ADDR*)vp_Tge;
    if (p_Src->iIdSec == p_Tge->iIdSec)
        return (double)p_Src->ulOfstRel - (double)p_Tge->ulOfstRel;
    return p_Src->iIdSec - p_Tge->iIdSec;
}


int
DsCompContentAddrPlus(const void *vp_Src, const void *vp_Tge, void *vp_Data)
{
    CONTENT_ADDR *p_Src = (CONTENT_ADDR*)vp_Src;
    CONTENT_ADDR *p_Tge = (CONTENT_ADDR*)vp_Tge;
    if (p_Src->iIdSec == p_Tge->iIdSec)
        return (double)p_Src->ulOfstRel - (double)p_Tge->ulOfstRel;
    return p_Src->iIdSec - p_Tge->iIdSec;
}


int8_t
DsInsertContentAddr(GTree *t_CtnAddr, CONTENT_ADDR *p_Addr, char *szPathFile)
{
    int8_t cRtnCode = SUCCESS;

    GPtrArray *a_Path = g_tree_lookup(t_CtnAddr, p_Addr);
    if (!a_Path) {
        a_Path = g_ptr_array_new();
        if (!a_Path)
            EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));
        g_tree_insert(t_CtnAddr, p_Addr, a_Path);
    } else
        free(p_Addr);
    g_ptr_array_add(a_Path, szPathFile);

EXIT:
    return cRtnCode;
}


gboolean
DsTravContentAddrCopy(gpointer gp_Key, gpointer gp_Val, gpointer gp_Tge)
{
    int8_t cRtnCode = SUCCESS;

    GTree *t_CtnAddr = (GTree*)gp_Tge;
    CONTENT_ADDR *p_AddrSrc = (CONTENT_ADDR*)gp_Key;
    GPtrArray *aStrTge = g_tree_lookup(t_CtnAddr, p_AddrSrc);
    if (!aStrTge) {
        CONTENT_ADDR *p_AddrTge = (CONTENT_ADDR*)malloc(sizeof(CONTENT_ADDR));
        if (!p_AddrTge)
            EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));
        aStrTge = g_ptr_array_new();
        if (!aStrTge)
            EXIT1(FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));
        p_AddrTge->iIdSec = p_AddrSrc->iIdSec;
        p_AddrTge->ulOfstRel = p_AddrSrc->ulOfstRel;
        g_tree_insert(t_CtnAddr, p_AddrTge, aStrTge);
    }

    GPtrArray *aStrSrc = (GPtrArray*)gp_Val;
    uint64_t ulIdx;
    for (ulIdx = 0 ; ulIdx < aStrSrc->len ; ulIdx++) {
        char *szPathFile = g_ptr_array_index(aStrSrc, ulIdx);
        g_ptr_array_add(aStrTge, szPathFile);
    }

EXIT:
    return (cRtnCode == SUCCESS)? false : true;
}


gboolean
DsTravContentAddrSize(gpointer gp_Key, gpointer gp_Val, gpointer gp_ulSize)
{
    uint64_t *p_ulSize = (uint64_t*)gp_ulSize;
    *p_ulSize += 1;

    return false;
}
