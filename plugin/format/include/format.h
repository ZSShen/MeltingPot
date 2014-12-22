#ifndef _FORMAT_H_
#define _FORMAT_H_


#include <stdint.h>
#include <glib.h>


enum {
    FMT_SUCCESS = 0,
    FMT_FAIL_FILE_IO = -1,
    FMT_FAIL_MEM_ALLOC = -2
};


/* This ds records the section text which fits the YARA format. */
typedef struct _FORMAT_TEXT_T {
    GString *gszSecStr;
    GString *gszSecCond;
    GString *gszComment;
} FORMAT_TEXT;


/* The exported interface to interact with this plugin. */
/* The function pointer type. */
typedef int8_t (*func_FmtInit) ();
typedef int8_t (*func_FmtDeinit) ();
typedef int8_t (*func_FmtAllocText) (FORMAT_TEXT**);
typedef int8_t (*func_FmtAppendSecStr) (FORMAT_TEXT, uint16_t*);
typedef int8_t (*func_FmtAppendSecCond) (FORMAT_TEXT, int32_t, uint64_t);
typedef int8_t (*func_FmtAppendComment) (FORMAT_TEXT, int32_t, uint64_t, GPtrArray*);
typedef int8_t (*func_FmtFinalize) (FORMAT_TEXT, char*);

/* The integrated structure to store exported functions. */
typedef struct _PLUGIN_FORMAT_T {
    void *hdle_Lib;
    func_FmtInit Init;
    func_FmtDeinit Deinit;
    func_FmtAllocText AllocText;
    func_FmtAppendSecStr AppendSecStr;
    func_FmtAppendSecCond AppendSecCond;
    func_FmtAppendComment AppendComment;
    func_FmtFinalize Finalize;
} PLUGIN_FORMAT;

/* The function name symbols. */
#define SYM_FMT_INIT                "FmtInit"
#define SYM_FMT_DEINIT              "FmtDeinit"
#define SYM_FMT_ALLOC_TEXT          "FmtAllocText"
#define SYM_FMT_APPEND_SEC_STR      "FmtAppendSecStr"
#define SYM_FMT_APPEND_SEC_COND     "FmtAppendSecCond"
#define SYM_FMT_APPEND_COMMENT      "FmtAppendComment"
#define SYM_FMT_FINALIZE            "FmtFinalize"


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
 * This function allocates and initializes the FORMAT_TEXT structure.
 *
 * @param pp_Text   The pointer to the pointer of to be initialized FORMAT_TEXT.
 *
 * @return status code
 */
int8_t
FmtAllocText(FORMAT_TEXT **pp_Text);


/**
 * This function appends the normalized byte array to the string section.
 *
 * @param p_Text    The pointer to the to be updated FORMAT_TEXT.
 * @param a_usCtn   The normalized byte array.
 *
 * @return status code
 */
int8_t
FmtAppendSecStr(FORMAT_TEXT *p_Text, uint16_t *a_usCtn);


/**
 * This function appends the string matching rule to the condition section.
 *
 * @param p_Text    The pointer to the to be updated FORMAT_TEXT.
 * @param iIdSec    The section id of the host file.
 * @param ulOfstRel The relative offset to the section starting address.
 *
 * @return status code
 */
int8_t
FmtAppendSecCond(FORMAT_TEXT *p_Text, int32_t iIdSec, uint64_t ulOfstRel);


/**
 * This function comments the details of candidate composition.
 *
 * @param p_Text    The pointer to the to be updated FORMAT_TEXT.
 * @param iIdSec    The section id of the host file.
 * @param ulOfstRel The relative offset to the section starting address.
 * @param a_Str     The array of pathnames.
 *
 * @return status code
 */
int8_t
FmtAppendComment(FORMAT_TEXT *p_Text, int32_t iIdSec, uint64_t ulOfstRel, 
                 GPtrArray *a_Str);


/**
 * This function produces the full text and outputs it to the designated path.
 *
 * @param p_Text    The pointer to the to be finalized FORMAT_TEXT.
 * @param szPathOut The output pathname.
 *
 * @return status code
 */
int8_t
FmtFinalize(FORMAT_TEXT *p_Text, char *szPathOut);


#endif
