#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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

    p_Text->gszSecStr = g_string_new(NULL);
    if (!p_Text->gszSecStr)
        EXIT1(FMT_FAIL_MEM_ALLOC, FREETEXT, "Error: %s.", strerror(errno));
    p_Text->gszSecCond = g_string_new(NULL);
    if (!p_Text->gszSecCond)
        EXIT1(FMT_FAIL_MEM_ALLOC, FREEGSTR, "Error: %s.", strerror(errno));
    p_Text->gszComment = g_string_new(NULL);
    if (!p_Text->gszComment)
        EXIT1(FMT_FAIL_MEM_ALLOC, FREEGSTR, "Error: %s.", strerror(errno));   
    goto EXIT;

FREEGSTR:
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
    if (p_Text)
        free(p_Text);

    return FMT_SUCCESS;
}
