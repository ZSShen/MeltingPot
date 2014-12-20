#ifndef _DATA_H_
#define _DATA_H_


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>
#include "spew.h"
#include "cluster.h"
#include "slice.h"
#include "similarity.h"


/* Useful number comparison macros. */
#define MIN(a, b)   (((a) < (b)) ? (a) : (b))
#define MAX(a, b)   (((a) > (b)) ? (a) : (b))


/* The addressing method to locate this binary block. */
typedef struct _CONTENT_ADDR_T {
    int32_t iIdSec;         /* The section id of the host file. (For certain file type) */
    uint64_t ulOfstRel;     /* The relative offset to the section starting address. */
} CONTENT_ADDR;


/* The distilled binary block which shows parts of the common features shared by the members 
   belonged to certain group. */
typedef struct _BLOCK_CAND_T {
    uint8_t ucCntNoise;     /* The number of noise bytes. */
    uint8_t ucCntWild;      /* The number of wildcard characters. */
    uint16_t *a_usCtn;      /* The normalized byte sequence. */
    GArray *a_CtnAddr;      /* The list of addressing methods to locate this block. */
    GTree *t_CtnAddr;       /* The map of addresses to locate this block. */
} BLOCK_CAND;


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


/* The structure recording the entire process context. */
typedef struct _CONTEXT_T {
    CONFIG *p_Conf;
    MELT_POT *p_Pot;
    PLUGIN_SLICE *plg_Slc;
    PLUGIN_SIMILARITY *plg_Sim;
} CONTEXT;


/* The structure recording the slice pair with score fitting the threshold. */
typedef struct _BIND_T {
    uint64_t ulIdSlcSrc;
    uint64_t ulIdSlcTge;
} BIND;


/**
 * The deinitialization function of c-string.
 *
 * @param gp_Str       The pointer to the target string.
 */
void
DsDeleteString(gpointer gp_Str);


/**
 * The deinitialization function of BIND structure.
 * 
 * @param gp_Bind      The pointer to the target structure.
 */
void
DsDeleteBind(gpointer gp_Bind);


/**
 * The deinitialization function of CONTENT_ADDR structure.
 * 
 * @param gp_CtnAddr   The pointer to the target structure.
 */
void
DsDeleteContentAddr(gpointer gp_CtnAddr);


/**
 * This deinitialization function of BLOCK_CAND structure.
 * 
 * @param gp_BlkCand    The pointer to the to be deallocated element.
 */
void
DsDeleteBlkCand(gpointer gp_BlkCand);


/**
 * The deinitialization function of hash key.
 * 
 * @param gp_Key        The pointer to the target key.
 */
void
DsDeleteHashKey(gpointer gp_Key);


/**
 * The deinitialization function of GROUP structure.
 * 
 * @param gp_Val        The pointer to the target structure.
 */
void
DsDeleteGroup(gpointer gp_Val);


/**
 * The deinitialization function of MELT_POT structure.
 * 
 * @param gp_Pot        The pointer to the target structure.
 */
void
DsDeleteMeltPot(gpointer gp_Pot);


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
 * @param plg_Slc       The handle of the file slicing plugin.
 * 
 * @return status code
 */
int8_t
DsNewMeltPot(MELT_POT **pp_Pot, PLUGIN_SLICE *plg_Slc);


/**
 * This function sorts the BLOCK_CAND structures with noise byte count.
 * 
 * @param vp_Src        The pointer to the pointer of source BlockCand structure.
 * @param vp_Tge        The pointer to the pointer of target one.
 * 
 * @return comparison order
 */
int
DsCompBlockCandNoise(const void *vp_Src, const void *vp_Tge);


/**
 * This function sorts the BLOCK_CAND structures with wildcard character count.
 * 
 * @param vp_Src        The pointer to the pointer of source BlockCand structure.
 * @param vp_Tge        The pointer to the pointer of target one.
 * 
 * @return comparison order
 */
int
DsCompBlockCandWildCard(const void *vp_Src, const void *vp_Tge);


/**
 * This function sorts the CONTENT_ADDR structures using section id as the
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
 * This function is specialized to compares the CONTENT_ADDR structures using 
 * section id as the first criteria and section offset as the second criteria.
 * 
 * @param vp_Src        The pointer to the pointer of source CONTENT_ADDR structure.
 * @param vp_Tge        The pointer to the pointer of target one.
 * @param vp_Data       The dummy pointer to fit the GLIB function prototype.
 * 
 * @return comparison order
 */
int
DsCompContentAddrPlus(const void *vp_Src, const void *vp_Tge, void *vp_Data);


/**
 * This callback function will be applied for GTree traversal and will be
 * used to copy the nodes of the caller tree to the target tree.
 * 
 * @param gp_Key        The pointer to the CONTENT_ADDR key.
 * @param gp_Val        The NULL pointer since we do not really insert value.
 * @param gp_Tge        The pointer to the target GTree.
 * 
 * @return working status
 */
gboolean
DsTravContentAddrCopy(gpointer gp_Key, gpointer gp_Val, gpointer gp_Tge);


#endif
