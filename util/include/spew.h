/**
 *   The MIT License (MIT)
 *   Copyright (C) 2014-2017 ZongXian Shen <andy.zsshen@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 */


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
