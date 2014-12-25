#ifndef _FORMAT_H_
#define _FORMAT_H_


#include <stdbool.h>
#include <stdint.h>
#include <glib.h>


/* The constants helping for YARA pattern creation. */
#define WILD_CARD_MARK          (0x100) /* The marker for wildcard character. */
#define EXTENSION_MASK          (0xff)  /* The mask used to extend char type to short. */

#define SIZE_INDENT_MAX         (64)    /* The maxumum indentation length. */
#define SIZE_HEX_LINE           (16)    /* The maximum number of bytes in a single line. */

#define PREFIX_PATTERN          "AUTO"  /* The prefix for pattern name. */
#define PREFIX_HEX_STRING       "SEQ"   /* The prefix for hex string name. */
#define CONJUNCTOR_OR           "or"    /* The logic "OR" conjunctor. */
#define SPACE_SUBS_TAB          "    "  /* The spaces substituting a tab. */


/* The exported interface to interact with this plugin. */
/* The function pointer type. */
typedef int8_t (*func_FmtInit) ();
typedef int8_t (*func_FmtDeinit) ();
typedef int8_t (*func_FmtPrint) (char*, uint64_t, GROUP*);

/* The integrated structure to store exported functions. */
typedef struct _PLUGIN_FORMAT_T {
    void *hdle_Lib;
    func_FmtInit Init;
    func_FmtDeinit Deinit;
    func_FmtPrint Print;
} PLUGIN_FORMAT;

/* The function name symbols. */
#define SYM_FMT_INIT                "FmtInit"
#define SYM_FMT_DEINIT              "FmtDeinit"
#define SYM_FMT_PRINT               "FmtPrint"


/**
 * This function initializes the pattern formatter plugin.
 *
 * @return status code
 */
int8_t
FmtInit();


/**
 * This function releases the resources allocated by pattern formatter plugin.
 *
 * @return status code
 */
int8_t
FmtDeinit();


/**
 * This function formats the common byte sequences of a given group into
 * the YARA format and outputs the pattern file.
 * 
 * @param szPathRoot        The root pathname of the output pattern folder.
 * @param ulIdxGrp          The group index.
 * @param p_Grp             The pointer to the GROUP structure.
 * 
 * @return status code.
 */
int8_t
FmtPrint(char *szPathRoot, uint64_t ulIdxGrp, GROUP *p_Grp);


#endif
