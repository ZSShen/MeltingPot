#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "spew.h"
#include "format.h"
#include "format_pe.h"


/*======================================================================*
 *                 Declaration for Internal Functions                   *
 *======================================================================*/
/**
 * This function appends the normalized byte array to the string section.
 *
 * @param p_Text    The pointer to the to be updated PATTERN_TEXT.
 * @param a_usCtn   The normalized byte array.
 * @param ucSize    The array size
 * 
 * @return status code
 */
int8_t
FmtAppendSecStr(PATTERN_TEXT *p_Text, uint16_t *a_usCtn, uint8_t ucSize);


/**
 * This function appends the string matching rule to the condition section.
 * Note that this function should be called after FmtAppendSecStr().
 * 
 * @param p_Text    The pointer to the to be updated PATTERN_TEXT.
 * @param iIdSec    The section id of the host file.
 * @param ulOfstRel The relative offset to the section starting address.
 * @param flagConj  The flags to guide the concatenation of "OR" conjunctor
 * 
 * @return status code
 */
int8_t
FmtAppendSecCond(PATTERN_TEXT *p_Text, int32_t iIdSec, uint64_t ulOfstRel,
                 HINT_CONJUNCT flagConj);


/**
 * This function comments the details of candidate composition.
 * Note that this function should be called after FmtAppendSecStr().
 * 
 * @param p_Text    The pointer to the to be updated PATTERN_TEXT.
 * @param iIdSec    The section id of the host file.
 * @param ulOfstRel The relative offset to the section starting address.
 * @param a_Str     The array of pathnames.
 *
 * @return status code
 */
int8_t
FmtAppendComment(PATTERN_TEXT *p_Text, int32_t iIdSec, uint64_t ulOfstRel, 
                 GPtrArray *a_Str);


/**
 * This function produces the full text and outputs it to the designated path.
 *
 * @param p_Text    The pointer to the to be finalized PATTERN_TEXT.
 * @param szPathOut The output pathname.
 *
 * @return status code
 */
int8_t
FmtFinalize(PATTERN_TEXT *p_Text, char *szPathOut);


/*======================================================================*
 *                Implementation for Exported Functions                 *
 *======================================================================*/
int8_t
FmtInit()
{
    return SUCCESS;
}


int8_t
FmtDeinit()
{
    return SUCCESS;
}


int8_t
FmtPrint(char *szPathRoot, uint64_t ulIdxGrp, GROUP *p_Grp)
{
    int8_t cRtnCode = SUCCESS;

    return cRtnCode;
}


/*======================================================================*
 *                Implementation for Internal Functions                 *
 *======================================================================*/
int8_t
FmtAppendSecStr(PATTERN_TEXT *p_Text, uint16_t *a_usCtn, uint8_t ucSize)
{
    GString *gszSecStr = p_Text->gszSecStr;

    uint64_t ulLenBefore = gszSecStr->len;
    g_string_append_printf(gszSecStr, "%s%s$%s_%d = { ", SPACE_SUBS_TAB,
                           SPACE_SUBS_TAB, PREFIX_HEX_STRING, p_Text->ucIdStr);
    uint64_t ulLenAfter = gszSecStr->len;

    /* Prepare the indentation. */
    char szIndent[SIZE_INDENT_MAX];
    memset(szIndent, 0, sizeof(char) * SIZE_INDENT_MAX);
    uint8_t ucIdx;
    for (ucIdx = 0 ; ucIdx < (ulLenAfter - ulLenBefore) ; ucIdx++)
        szIndent[ucIdx] = ' ';

    /* Print the hex byte block as plaintext string. */
    for (ucIdx = 0 ; ucIdx < ucSize ; ucIdx++) {
        if (a_usCtn[ucIdx] != WILD_CARD_MARK)
            g_string_append_printf(gszSecStr, "%02x ", a_usCtn[ucIdx] & EXTENSION_MASK);
        else
            g_string_append(gszSecStr, "?? ");

        /* Newline if the number of writtern bytes exceeding the line boundary. */
        if ((ucIdx % SIZE_HEX_LINE == SIZE_HEX_LINE - 1) && (ucIdx != ucSize - 1))
            g_string_append_printf(gszSecStr, "\n%s", szIndent);
    }

    g_string_append(gszSecStr, "}\n\n");
    p_Text->ucIdStr++;

    return SUCCESS;
}


int8_t
FmtAppendSecCond(PATTERN_TEXT *p_Text, int32_t iIdSec, uint64_t ulOfstRel,
                 HINT_CONJUNCT flagConj)
{
    GString *gszSecCond = p_Text->gszSecCond;
    uint8_t ucIdStr = p_Text->ucIdStr - 1;

    g_string_append_printf(gszSecCond, "%s%s$%s_%d at %s.%s[%d].%s + 0x%lx",
                           SPACE_SUBS_TAB, SPACE_SUBS_TAB, PREFIX_HEX_STRING,
                           ucIdStr, IMPORT_MODULE_PE, TAG_SECTION,
                           iIdSec, TAG_RAW_DATA_OFFSET, ulOfstRel);

    if (flagConj.bOrBlk)
        g_string_append_printf(gszSecCond, " %s\n", CONJUNCTOR_OR);
    else
        g_string_append(gszSecCond, "\n");

    if (flagConj.bOrSec)
        g_string_append_printf(gszSecCond, "%s%s%s\n", SPACE_SUBS_TAB,
                               SPACE_SUBS_TAB, CONJUNCTOR_OR);

    return SUCCESS;
}


int8_t
FmtAppendComment(PATTERN_TEXT *p_Text, int32_t iIdSec, uint64_t ulOfstRel, 
                 GPtrArray *a_Str)
{
    GString *gszComt = p_Text->gszComt;
    uint8_t ucIdStr = p_Text->ucIdStr - 1;

    return SUCCESS;
}
