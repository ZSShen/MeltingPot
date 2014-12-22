#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "spew.h"
#include "format.h"
#include "format_pe.h"


int8_t
FmtInit()
{
    return FMT_SUCCESS;
}


int8_t
FmtDeinit()
{
    return FMT_SUCCESS;
}


int8_t
FmtAllocText(FORMAT_TEXT **pp_Text)
{
    int cRtnCode = FMT_SUCCESS;

    *pp_Text = (FORMAT_TEXT*)malloc(sizeof(FORMAT_TEXT));
    if (!(*pp_Text))
        EXIT1(FMT_FAIL_MEM_ALLOC, EXIT, "Error: %s.", strerror(errno));

    FORMAT_TEXT *p_Text = *pp_Text;
    p_Text->gszSecStr = NULL;
    p_Text->gszSecCond = NULL;
    p_Text->gszComment = NULL;
    p_Text->gszFullPtn = NULL;

    p_Text->gszSecStr = g_string_new(NULL);
    if (!p_Text->gszSecStr)
        EXIT1(FMT_FAIL_MEM_ALLOC, FREETEXT, "Error: %s.", strerror(errno));
    p_Text->gszSecCond = g_string_new(NULL);
    if (!p_Text->gszSecCond)
        EXIT1(FMT_FAIL_MEM_ALLOC, FREEGSTR, "Error: %s.", strerror(errno));
    p_Text->gszComment = g_string_new(NULL);
    if (!p_Text->gszComment)
        EXIT1(FMT_FAIL_MEM_ALLOC, FREEGSTR, "Error: %s.", strerror(errno));   
    p_Text->gszFullPtn = g_string_new(NULL);
    if (!p_Text->gszFullPtn)
        EXIT1(FMT_FAIL_MEM_ALLOC, FREEGSTR, "Error: %s.", strerror(errno));

    p_Text->ucIdStr = 0;
    p_Text->ucIdCond = 0;
    goto EXIT;

FREEGSTR:
    if (p_Text->gszComment)
        g_string_free(p_Text->gszComment, true);
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


int8_t
FmtDeallocText(FORMAT_TEXT *p_Text)
{
    if (p_Text->gszSecStr)
        g_string_free(p_Text->gszSecStr, true);
    if (p_Text->gszSecCond)
        g_string_free(p_Text->gszSecCond, true);
    if (p_Text->gszComment)
        g_string_free(p_Text->gszComment, true);
    if (p_Text->gszFullPtn)
        g_string_free(p_Text->gszFullPtn, true);    
    if (p_Text)
        free(p_Text);

    return FMT_SUCCESS;
}


int8_t
FmtAppendSecStr(FORMAT_TEXT *p_Text, uint16_t *a_usCtn, uint8_t ucSize)
{
    GString *gszSecStr = p_Text->gszSecStr;

    uint64_t ulLenBefore = gszSecStr->len;
    g_string_append_printf(gszSecStr, "%s%s$%s_%d = { ", SPACE_SUBS_TAB,
                           SPACE_SUBS_TAB, PREFIX_HEX_STRING, p_Text->ucIdStr);
    uint64_t ulLenAfter = gszSecStr->len;

    /* Prepare the indentation. */
    char szIndent[BUF_SIZE_INDENT];
    memset(szIndent, 0, sizeof(char) * BUF_SIZE_INDENT);
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
        if ((ucIdx % HEX_CHUNK_SIZE == HEX_CHUNK_SIZE - 1) && (ucIdx != ucSize - 1))
            g_string_append_printf(gszSecStr, "\n%s", szIndent);
    }

    g_string_append(gszSecStr, "}\n\n");
    p_Text->ucIdStr++;

    return FMT_SUCCESS;
}


int8_t
FmtAppendSecCond(FORMAT_TEXT *p_Text, int32_t iIdSec, uint64_t ulOfstRel,
                 HINT_CONJUNCT flagConj)
{
    GString *gszSecCond = p_Text->gszSecCond;

    g_string_append_printf(gszSecCond, "%s%s$%s_%d at %s.%s[%d].%s + 0x%lx",
                           SPACE_SUBS_TAB, SPACE_SUBS_TAB, PREFIX_HEX_STRING,
                           p_Text->ucIdCond, IMPORT_MODULE_PE, TAG_SECTION,
                           iIdSec, TAG_RAW_DATA_OFFSET, ulOfstRel);

    if (flagConj.bOrBlk)
        g_string_append_printf(gszSecCond, " %s\n", CONJUNCTOR_OR);
    else
        g_string_append(gszSecCond, "\n");

    if (flagConj.bOrSec)
        g_string_append_printf(gszSecCond, "%s%s%s\n", SPACE_SUBS_TAB,
                               SPACE_SUBS_TAB, CONJUNCTOR_OR);
    p_Text->ucIdCond++;

    return FMT_SUCCESS;
}
