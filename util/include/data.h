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


#ifndef _DATA_H_
#define _DATA_H_


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>


/* Useful number comparison macros. */
#define MIN(a, b)   (((a) < (b)) ? (a) : (b))
#define MAX(a, b)   (((a) > (b)) ? (a) : (b))


/* The addressing method to locate this binary block. */
typedef struct _CONTENT_ADDR_T {
    int32_t iIdSec;         /* The section id of the host file. (For certain file type) */
    uint64_t ulOfstRel;     /* The relative offset to the section starting address. */
} CONTENT_ADDR;


/* The distilled binary block which shows parts of the common byte sequences shared by
   the members belonged to certain group. */
typedef struct _BLOCK_CAND_T {
    uint8_t ucCntNoise;     /* The number of noise bytes. */
    uint8_t ucCntWild;      /* The number of wildcard characters. */
    uint8_t ucSizeCtn;      /* The block content size. */
    uint16_t *a_usCtn;      /* The normalized byte sequence. */
    GTree *t_CtnAddr;       /* The map of addresses to locate this block. */
} BLOCK_CAND;


/* Slice is a file block containing designated number of bytes. This structure records 
   information to locate a slice. */
typedef struct _SLICE_T {
    int32_t iIdSec;         /* The section id of the host file. (For certain file type) */
    uint64_t ulOfstAbs;     /* The absolute offset of the host file. */
    uint64_t ulOfstRel;     /* The relative offset to the section starting address.*/
    uint16_t usSize;        /* The slice size. */
    union {
        uint64_t ulIdSlc;   /* The logic id for memorization. */
        uint64_t ulIdGrp;   /* The group id (after correlation) this slice belonging to.*/
    };
    char *szPathFile;       /* The aboslute path of the host file. */
} SLICE;


/* The structure recording the information shared by the members belonged to certain group. */
typedef struct _GROUP_T {
    uint64_t ulIdGrp;       /* The group id. */
    GArray *a_Mbr;          /* The list of group members. */
    GPtrArray *a_BlkCand;   /* The list of distilled binary blocks. */
} GROUP;


/* The structure recording the complete information of clustering progress.  */
typedef struct _MELT_POT_T {
    GPtrArray *a_Path;      /* The list of pathnames belonged to the given sample set. */ 
    GPtrArray *a_Hash;      /* The list of slice hashes. */
    GPtrArray *a_Slc;       /* The list of detailed slice information. */
    GHashTable *h_Grp;      /* The hash table storing the groups. */
} MELT_POT;


/* The structure recording the slice pair with score fitting the threshold. */
typedef struct _BIND_T {
    uint64_t ulIdSlcSrc;
    uint64_t ulIdSlcTge;
} BIND;


/* This structure recording the section texts which fits the YARA format. */
typedef struct _PATTERN_TEXT_T {
    GString *gszSecStr;
    GString *gszSecCond;
    GString *gszComt;
    GString *gszFullPtn;
} PATTERN_TEXT;


/**
 * The deinitialization function of c-string.
 * Note: It is the element deletion function of GPtrArray: a_Path, a_Hash.
 * 
 * @param gp_Str       The pointer to the target string.
 */
void
DsDeleteString(gpointer gp_Str);


/**
 * The deinitialization function of BIND structure.
 * Note: It is the element deletion function of GPtrArray: a_Bind.
 * 
 * @param gp_Bind      The pointer to the target structure.
 */
void
DsDeleteBind(gpointer gp_Bind);


/**
 * The deinitialization function of an array of string pointer.
 * Note: It is the value deletion function of GTree: t_CtnAddr.
 * 
 * @param gp_APath   The pointer to the target array.
 */
void
DsDeleteArrayPath(gpointer gp_APath);


/**
 * The deinitialization function of CONTENT_ADDR structure.
 * Note: It is the key deletion function of GTree: t_CtnAddr.
 * 
 * @param gp_CtnAddr   The pointer to the target structure.
 */
void
DsDeleteContentAddr(gpointer gp_CtnAddr);


/**
 * The deinitialization function of BLOCK_CAND structure.
 * Note: It is the element deletion function of GPtrArray: a_BlkCand.
 * 
 * @param gp_BlkCand    The pointer to the target structure.
 */
void
DsDeleteBlkCand(gpointer gp_BlkCand);


/**
 * The deinitialization function of hash key.
 * Note: It is the key deletion function of GHashTable: h_Grp.
 * 
 * @param gp_Key        The pointer to the target key.
 */
void
DsDeleteHashKey(gpointer gp_Key);


/**
 * The deinitialization function of SLICE structure.
 * Note: It is the element deletion function of GPtrArray: a_Slc.
 * 
 * @param gp_Slc        The pointer to the target structure.
 */
void
DsDeleteSlice(gpointer gp_Slc);


/**
 * The deinitialization function of GROUP structure.
 * Note: It is the value deletion function of GHashTable: h_Grp.
 * 
 * @param gp_Grp        The pointer to the target structure.
 */
void
DsDeleteGroup(gpointer gp_Grp);


/**
 * The deinitialization function of MELT_POT structure.
 * 
 * @param gp_Pot        The pointer to the target structure.
 */
void
DsDeleteMeltPot(gpointer gp_Pot);


/**
 * The deinitialization function of PATTERN_TEXT structure.
 * 
 * @param p_Text        The pointer to the target structure.
 * 
 */
void
DsDeletePatternText(PATTERN_TEXT *p_Text);


/**
 * The initialization function of BLOCK_CAND structure.
 * 
 * @param pp_BlkCand    The pointer to the pointer of target structure.
 * @param usSizeCtn     The size of the block content.
 * 
 * @return status code
 */
int8_t
DsNewBlockCand(BLOCK_CAND **pp_BlkCand, uint8_t usSizeCtn);


/**
 * The initialization function of GROUP structure.
 *
 * @param pp_Grp        The pointer to the pointer of target structure.
 * 
 * @return status code
 */
int8_t
DsNewGroup(GROUP **pp_Grp);


/**
 * The initialization function of MELT_POT structure.
 * 
 * @param pp_Pot        The pointer to the pointer of target structure.
 * 
 * @return status code
 */
int8_t
DsNewMeltPot(MELT_POT **pp_Pot);


/**
 * The initialization function of PATTERN_TEXT structure.
 *
 * @param pp_Text       The pointer to the pointer of target structure.
 *
 * @return status code
 */
int8_t
DsNewPatternText(PATTERN_TEXT **pp_Text);


/**
 * This function sorts the BLOCK_CAND structures via noise byte count.
 * 
 * @param vp_Src        The pointer to the pointer of source BLOCK_CAND structure.
 * @param vp_Tge        The pointer to the pointer of target one.
 * 
 * @return comparison order
 */
int
DsCompBlockCandNoise(const void *vp_Src, const void *vp_Tge);


/**
 * This function sorts the BLOCK_CAND structures via wildcard character count.
 * 
 * @param vp_Src        The pointer to the pointer of source BLOCK_CAND structure.
 * @param vp_Tge        The pointer to the pointer of target one.
 * 
 * @return comparison order
 */
int
DsCompBlockCandWildCard(const void *vp_Src, const void *vp_Tge);


/**
 * This function sorts the CONTENT_ADDR structures with section id as the
 * first criteria and section offset as the second criteria.
 * 
 * @param vp_Src        The pointer to the pointer of source CONTENT_ADDR structure.
 * @param vp_Tge        The pointer to the pointer of target one.
 * 
 * @return comparison order
 */
int
DsCompContentAddr(const void *vp_Src, const void *vp_Tge);


/**
 * This function is the same as DsCompContentAddr() but with parameter list
 * specialiezed for GTree.
 * 
 * @param vp_Src        The pointer to the pointer of source CONTENT_ADDR structure.
 * @param vp_Tge        The pointer to the pointer of target one.
 * @param vp_Data       The dummy pointer to fit the GTree function prototype.
 * 
 * @return comparison order
 */
int
DsCompContentAddrPlus(const void *vp_Src, const void *vp_Tge, void *vp_Data);


/**
 * This function inserts a key value pair into GTree with CONTENT_ADDR as
 * key and pathname as an element of the value array.
 * 
 * @param t_CtnAddr     The pointer to the to be inserted GTree structure.
 * @param p_Addr        The pointer to the CONTENT_ADDR structure.
 * @param szPathFile    The pointer to the pathname.
 * 
 * @return status code
 */
int8_t
DsInsertContentAddr(GTree *t_CtnAddr, CONTENT_ADDR *p_Addr, char *szPathFile);


/**
 * This callback function will be applied for GTree traversal and will be
 * used to copy the nodes of the caller tree to the target tree.
 * 
 * @param gp_Key        The pointer to the key: CONTENT_ADDR key.
 * @param gp_Val        The pointer to the value: pathname array.
 * @param gp_Tge        The pointer to the target GTree structure.
 * 
 * @return traversal control flag
 */
gboolean
DsTravContentAddrCopy(gpointer gp_Key, gpointer gp_Val, gpointer gp_Tge);


/**
 * This callback function will be applied for GTree traversal and will be
 * used to count the tree size.
 * 
 * @param gp_Key        The pointer to the key: CONTENT_ADDR key.
 * @param gp_Val        The pointer to the value: pathname array.
 * @param gp_ulSize     The pointer to the to be updated size variable.
 * 
 * @return traversal control flag
 */
gboolean
DsTravContentAddrSize(gpointer gp_Key, gpointer gp_Val, gpointer gp_ulSize);


#endif
