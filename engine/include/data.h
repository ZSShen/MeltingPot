#ifndef _DATA_H_
#define _DATA_H_


#include <stdint.h>
#include <glib.h>
#include "cluster.h"
#include "slice.h"
#include "similarity.h"


/* The addressing method to locate this binary block. */
typedef struct _CONTENT_ADDR_T {
    int32_t iIdSec;         /* The section id of the host file. (For certain file type) */
    uint64_t ulOfstRel;     /* The relative offset to the section starting address. */
} CONTENT_ADDR;


/* The distilled binary block which shows parts of the common features shared by the members 
   belonged to certain group. */
typedef struct _BLOCK_CAND_T {
    uint16_t *p_usCont;     /* The normalized byte sequence. */
    GPtrArray *a_ContAddr;  /* The list of addressing methods to locate this block. */
} BLOCK_CAND;


/* The structure recording the information shared by the members belonged to certain group. */
typedef struct _GROUP_T {
    uint16_t usCntBlk;      /* The number of distilled binary blocks. */
    uint64_t ulIdGrp;       /* The group id. */
    uint64_t ulSizeGrp;     /* The number of group members. */
    GPtrArray *a_Mbr;       /* The list of grouped members. */
    GPtrArray *a_BlkCand;   /* The list of distilled binary blocks. */
} GROUP;


/* The structure recording the complete information of clustering progress.  */
typedef struct _MELT_POT_T {
    uint64_t ulCntGrp;      /* The number of groups. */
    GPtrArray *a_Name;      /* The list of filenames belonged to the given sample set. */ 
    GPtrArray *a_Hash;      /* The list of slice hashes. */
    GPtrArray *a_Slc;       /* The list of detailed slice information. */
    GHashTable *a_Grp;      /* The hash table storing the groups. */
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
 * This function hints the Glib to deallocate file name elements.
 *
 * @param gp_Name       The pointer to the to be deallocated element.
 */
void
DsFreeNameArray(gpointer gp_Name);


/**
 * This function hints the Glib to deallocate hash elements.
 * 
 * @param gp_Hash      The pointer to the to be deallocated element.
 */ 
void
DsFreeHashArray(gpointer gp_Hash);


/**
 * This function hints the Glib to deallocate bind elements.
 * 
 * @param gp_Bind      The pointer to the to be deallocated element.
 */
void
DsFreeBindArray(gpointer gp_Bind);


#endif
