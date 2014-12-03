#ifndef _DATA_H_
#define _DATA_H_


#include <stdint.h>
#include <glib.h>


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


/* The structure recording the complete information of clustering process.  */
typedef struct _MELT_PLOT_T {
    uint64_t ulCntGrp;      /* The number of groups. */
    GPtrArray *a_Hash;      /* The list of slice hashes. */
    GPtrArray *a_Slc        /* The list of detailed slice information.*/
    GHashTable *a_Grp;      /* The hash table aiding to access group information. */
} MELT_PLOT;


#endif
