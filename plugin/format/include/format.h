#ifndef _FORMAT_H_
#define _FORMAT_H_


#include <stdint.h>
#include <glib.h>


enum {
    FRM_SUCCESS = 0,
    FRM_FAIL_FILE_IO = -1,
    FRM_FAIL_MEM_ALLOC = -2
};


/* This ds records the section text which fits the YARA format. */
typedef struct _FORMAT_TEXT_T {
    GString *gszSecStr;
    GString *gszSecCond;
    GString *gszComment;
} FORMAT_TEXT;


/* The exported interface to interact with this plugin. */
/* The function pointer type. */
typedef int8_t (*func_FrmInit) ();
typedef int8_t (*func_FrmDeinit) ();
typedef int8_t (*func_FrmAllocText) (FORMAT_TEXT**);
typedef int8_t (*func_FrmAppendSecStr) (FORMAT_TEXT, uint16_t*);
typedef int8_t (*func_FrmAppendSecCond) (FORMAT_TEXT, int32_t, uint64_t);
typedef int8_t (*func_FrmAppendComment) (FORMAT_TEXT, int32_t, uint64_t, GPtrArray*);
typedef int8_t (*func_FrmFinalize) (FORMAT_TEXT, char*);

/* The integrated structure to store exported functions. */
typedef struct _PLUGIN_FORMAT_T {
    void *hdle_Lib;
    func_FrmInit Init;
    func_FrmDeinit Deinit;
    func_FrmAllocText AllocText;
    func_FrmAppendSecStr AppendSecStr;
    func_FrmAppendSecCond AppendSecCond;
    func_FrmAppendComment AppendComment;
    func_FrmFinalize Finalize;
} PLUGIN_FORMAT;

/* The function name symbols. */
#define SYM_FRM_INIT                "FrmInit"
#define SYM_FRM_DEINIT              "FrmDeinit"
#define SYM_FRM_ALLOC_TEXT          "FrmAllocText"
#define SYM_FRM_APPEND_SEC_STR      "FrmAppendSecStr"
#define SYM_FRM_APPEND_SEC_COND     "FrmAppendSecCond"
#define SYM_FRM_APPEND_COMMENT      "FrmAppendComment"
#define SYM_FRM_FINALIZE            "FrmFinalize"


/**
 * This function initializes the pattern formatter plugin.
 *
 * @return status code
 */
int8_t
FrmInit();


/**
 * This function releases the resources allocated by pattern formatter plugin.
 *
 * @return status code
 */
int8_t
FrmDeinit();


/**
 * This function allocates and initializes the FORMAT_TEXT structure.
 *
 * @param pp_Text   The pointer to the pointer of to be initialized FORMAT_TEXT.
 *
 * @return status code
 */
int8_t
FrmAllocText(FORMAT_TEXT **pp_Text);


/**
 * This function appends the normalized byte array to the string section.
 *
 * @param p_Text    The pointer to the to be updated FORMAT_TEXT.
 * @param a_usCtn   The normalized byte array.
 *
 * @return status code
 */
int8_t
FrmAppendSecStr(FORMAT_TEXT *p_Text, uint16_t *a_usCtn);


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
FrmAppendSecCond(FORMAT_TEXT *p_Text, int32_t iIdSec, uint64_t ulOfstRel);


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
FrmAppendComment(FORMAT_TEXT *p_Text, int32_t iIdSec, uint64_t ulOfstRel, 
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
FrmFinalize(FORMAT_TEXT *p_Text, char *szPathOut);


#endif
