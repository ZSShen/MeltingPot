#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "except.h"
#include "spew.h"
#include "format.h"
#include "format_pe.h"


/*======================================================================*
 *                 Declaration for Internal Functions                   *
 *======================================================================*/
/**
 * This function appends the normalized byte array to the string section.
 *
 * @param p_Text        The pointer to the PATTERN_TEXT structure.
 * @param a_usCtn       The normalized byte array.
 * @param ucSize        The array size
 * 
 * @return status code
 */
int8_t
_FmtAppendSecStr(PATTERN_TEXT *p_Text, uint16_t *a_usCtn, uint8_t ucSize);


/**
 * This callback function appends the string matching rule to the condition section.
 * 
 * @param gp_Key        The pointer to the key: CONTENT_ADDR key.
 * @param gp_Val        The pointer to the value: pathname array.
 * @param p_Trav        The pointer to the TRAV structure.
 * 
 * @return traversal control flag
 */
gboolean
_FmtAppendSecCond(gpointer *gp_Key, gpointer *gp_Val, TRAV *p_Trav);


/**
 * This function shows the details of a group composition.
 * 
 * @param gp_Key        The pointer to the key: CONTENT_ADDR key.
 * @param gp_Val        The pointer to the value: pathname array.
 * @param p_Trav        The pointer to the TRAV structure.
 * 
 * @return traversal control flag
 */
gboolean
_FmtAppendComment(gpointer *gp_Key, gpointer *gp_Val, TRAV *p_Trav);


/**
 * This function produces the full text and outputs it to the designated path.
 *
 * @param p_Text        The pointer to the PATTERN_TEXT structure.
 * @param szPathOut     The output pathname.
 *
 * @return status code
 */
int8_t
_FmtFinalize(PATTERN_TEXT *p_Text, char *szPathOut);


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
_FmtAppendSecStr(PATTERN_TEXT *p_Text, uint16_t *a_usCtn, uint8_t ucSize)
{
    GString *gszSecStr = p_Text->gszSecStr;

    uint64_t ulLenBefore = gszSecStr->len;
    g_string_append_printf(gszSecStr, "%s%s$%s_%d = { ", SPACE_SUBS_TAB,
    SPACE_SUBS_TAB, PREFIX_HEX_STRING, p_Text->ucIdxStr);
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

    return SUCCESS;
}


gboolean
_FmtAppendSecCond(gpointer *gp_Key, gpointer *gp_Val, TRAV *p_Trav)
{
    PATTERN_TEXT *p_Text = p_Trav->p_Text;
    GString *gszSecCond = p_Text->gszSecCond;
    CONTENT_ADDR *p_Addr = (CONTENT_ADDR*)gp_Key;
    uint8_t ucIdxStr = p_Text->ucIdxStr;
    int32_t iIdSec = p_Addr->iIdSec;
    uint64_t ulOfstRel = p_Addr->ulOfstRel;

    g_string_append_printf(gszSecCond, "%s%s$%s_%d at %s.%s[%d].%s + 0x%lx",
    SPACE_SUBS_TAB, SPACE_SUBS_TAB, PREFIX_HEX_STRING, ucIdxStr, IMPORT_MODULE_PE,
    TAG_SECTION, iIdSec, TAG_RAW_DATA_OFFSET, ulOfstRel);

    if ((p_Trav->ulIdxCond != p_Trav->ulCntCond) && (p_Trav->ulIdxCond != 1))
        g_string_append_printf(gszSecCond, " %s\n", CONJUNCTOR_OR);
    else
        g_string_append(gszSecCond, "\n");

    if (p_Trav->ucIdxBlk != p_Trav->ucCntBlk)
        g_string_append_printf(gszSecCond, "%s%s%s\n", SPACE_SUBS_TAB,
        SPACE_SUBS_TAB, CONJUNCTOR_OR);

    return false;
}


gboolean
_FmtAppendComment(gpointer *gp_Key, gpointer *gp_Val, TRAV *p_Trav)
{
    PATTERN_TEXT *p_Text = p_Trav->p_Text;
    GString *gszComt = p_Text->gszComt;
    CONTENT_ADDR *p_Addr = (CONTENT_ADDR*)gp_Key;
    GPtrArray *a_Path = (GPtrArray*)gp_Val;
    uint8_t ucIdxStr = p_Text->ucIdxStr;
    int32_t iIdSec = p_Addr->iIdSec;
    uint64_t ulOfstRel = p_Addr->ulOfstRel;

    if (!p_Trav->bDeclare) {
        g_string_append_printf(gszComt, "$%s_%d %s:\n", PREFIX_HEX_STRING,
        ucIdxStr, COMMENT_CONTRIBUTE);
        p_Trav->bDeclare = true;
    }

    g_string_append_printf(gszComt, "%s%s[%d] %s 0x%lx:", SPACE_SUBS_TAB,
    TAG_SECTION, iIdSec, COMMENT_RELATIVE_OFFSET, ulOfstRel);

    uint64_t ulLen = a_Path->len;
    uint64_t ulIdx;
    for (ulIdx = 0 ; ulIdx < ulLen ; ulIdx++) {
        char *szPath = g_ptr_array_index(a_Path, ulIdx);
        g_string_append_printf(gszComt, "%s%s%s\n", SPACE_SUBS_TAB,
        SPACE_SUBS_TAB, szPath);
    }

    if (p_Trav->ulIdxCond == p_Trav->ulCntCond)
        g_string_append(gszComt, "\n");

    return false;
}
