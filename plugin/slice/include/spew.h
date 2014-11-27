#ifndef _SPEW_H_
#define _SPEW_H_


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>


#define MSG_BUF_SIZE   1024


/* Wrapper for log function. */
#define Spew                         SpewMessage(__FILE__, __LINE__, __FUNCTION__,
#define Spew0(p0)                    Spew p0)
#define Spew1(p0, p1)                Spew p0, p1)
#define Spew2(p0, p1, p2)            Spew p0, p1, p2)
#define Spew3(p0, p1, p2, p3)        Spew p0, p1, p2, p3)


/**
 * This function generates the log message.
 *
 * @param szPathFile    The pathname of the source file.
 * @param iLineFile     The line number of the source file.
 * @param szNameFunc    The function name.
 * @param szFormat      The string formatter.
 * @param ...           The parameters of the string formatter.
 */
void
SpewMessage(const char *szPathFile, const int32_t iLineFile, const char *szNameFunc,
            const char *szFormat, ...);


#endif
