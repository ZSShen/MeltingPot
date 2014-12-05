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
#define SPEW                         SpewMessage(__FILE__, __LINE__, __FUNCTION__,
#define SPEW0(p0)                    SPEW p0)
#define SPEW1(p0, p1)                SPEW p0, p1)
#define SPEW2(p0, p1, p2)            SPEW p0, p1, p2)


/* Wrapper for early return mechanism. */
#define SETNGO(rc, label)            cRtnCode = rc;                     \
                                     goto label;

#define EXIT0(rc, label, p0)         do {                               \
                                        SPEW0(p0);                      \
                                        SETNGO(rc, label);              \
                                     } while (0);

#define EXIT1(rc, label, p0, p1)     do {                               \
                                        SPEW1(p0, p1);                  \
                                        SETNGO(rc, label);              \
                                     } while (0);
                                    
#define EXIT2(rc, label, p0, p1, p2) do {                               \
                                        SPEW2(p0, p1, p2);              \
                                        SETNGO(rc, label);              \
                                     } while (0);
                                     
#define EXITQ(rc, label)             do {                               \
                                        SETNGO(rc, label);              \
                                     } while (0);

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
